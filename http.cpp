#include "http.hpp"
#include <cstring>
#include "utility_http.hpp"
#include <sstream>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "errors.hpp"

namespace http {

const char	DQUOTE = '\"';

// HTTP-message = start-line *( header-field CRLF ) CRLF [ message-body ]
bool	parse_request(Request& req, std::string const &data) {
  size_t pos = 0;
  req.content_length = -1;
  req.close = false;
  req.code = NoError;

  pos += parse_request_line(req, data, pos, req.code);
  if (req.code != NoError)
	return true;
  req.serv_config = chooseServer(req.url);
  pos += parse_headers(req.headers, data, pos, req.code);
  if (req.code != NoError)
	return true;
  header_analysis(req, req.headers, req.code);
  if (req.code != NoError)
	return true;
  calculate_length_message(req, req.code);
  if (req.code != NoError)
	return true;
  if (req.code != NoError) {
	return true;
  }
  if (!check_config(req))
	return true;
  if (req.content_length != 0) {
	bool answer = read_body(req, data, pos, req.code);
	if (req.code != NoError && req.code != StatusOK && req.code != StatusCreated) {
	  req.representation.clear();
	  return true;
	}
	return answer;
  }
  return true;
}

void	parse_request_target(url::URL &url, Method &, const std::string &str,
						  StatusCode &code) {
  if (str.length() > 8000) {
	code = StatusRequestURITooLong;
	return;
  }
  if (!parse_origin_form(url, str) && !parse_absolute_uri(url, str)) {
	code = StatusBadRequest;
  }
}

void	parse_and_validate_method(Method &m, const std::string &str, StatusCode &code) {
  static std::map<const std::string, Method> methods = {
	  {"GET", GET},
	  {"POST", POST},
	  {"DELETE", DELETE},
  };
  if (methods.count(str)) {
    m = methods[str];
  } else {
    code = StatusNotImplemented;
  }
}

// request-line = method SP request-target SP HTTP-version CRLF
size_t parse_request_line(Request &r, std::string const &data, size_t begin, StatusCode &code) {
  size_t pos = begin;

  //skip first CRLF (3.5)
  if (data.length() > 1 && data.compare(pos, 2, "\r\n") == 0)
  {
    pos += skip_crlf(data, pos, code);
    if (code != NoError)
	  return 0;
  }
  std::string method;
  pos += get_token(method, data, pos, code);
  if (code != NoError)
	return 0;
  parse_and_validate_method(r.method, method, code);
  if (code != NoError)
	return 0;
  pos += skip_space(data, pos, SP, code);
  std::string request_target;
  pos += get_request_target(request_target, data, pos, code);
  if (code != NoError)
	return 0;
  parse_request_target(r.url, r.method, request_target, code);
  if (code != NoError)
	return 0;
  pos += skip_space(data, pos, SP, code);
  if (code != NoError)
	return 0;
  pos += get_http_version(r.proto, data, pos, code);
  if (code != NoError)
	return 0;
  if (r.proto != "HTTP/1.1") {
	return code = StatusHTTPVersionNotSupported;
  }
  pos += skip_crlf(data, pos, code);
  if (code != NoError)
	return 0;
  return pos - begin;
}

// header-field = field-name ":" OWS field-value OWS
size_t parse_headers(Headers &dst, std::string const &data, size_t begin, StatusCode &code) {
  size_t pos = begin;
  std::string field_name;
  std::string field_value;

  if (data[pos] == SP) {
	return code = StatusBadRequest;
  }
  while (pos + 1 < data.length() && data.compare(pos, 2, "\r\n") != 0) {
    // field-name = token
	pos += get_token(field_name, data, pos, code);
	if (code != NoError)
	  return 0;
	tolower(field_name);
	if (data[pos] != ':') {
	  return code = StatusBadRequest;
	}
	++pos;
	pos += skip_space(data, pos, OWS, code);
	if (code != NoError)
	  return 0;
	// field-value = *( field-content / obs-fold )
	// field-content = field-vchar [ 1*( SP / HTAB ) field-vchar ]
	// field-vchar = VCHAR / obs-text

	// obs-fold = CRLF 1*( SP / HTAB )
	bool obs_fold;
	do {
	  size_t local_size = 0;
	  while (data[pos + local_size] < 0 || isgraph(data[pos + local_size]) ||
	  isblank(data[pos + local_size]))
		++local_size;
	  if (pos + local_size + 2 >= data.length() ||
	  data.compare(pos+local_size, 2, "\r\n") != 0) {
		return code = StatusBadRequest;
	  }
	  field_value.append(data, pos, local_size);
	  field_value.append(1, ' ');
	  pos += local_size;
	  pos += 2;
	  if (isblank(data[pos]))
	    obs_fold = true;
	  else
	    obs_fold = false;
	  while (isblank(data[pos]))
	    ++pos;
	} while (obs_fold);
	// OWS
	while (isblank(*field_value.rbegin()))
	  field_value.erase(field_value.end() - 1);

	if (dst.count(field_name) == 1) {
	  if (field_name == "host") {
		return code = StatusBadRequest;
	  }
	  dst[field_name].append(",");
	}
	dst[field_name].append(field_value);
	field_value.clear();
	field_name.clear();
  }
  pos += skip_crlf(data, pos, code);
  if (code != NoError)
	return 0;
  return pos - begin;
}

void calculate_length_message(Request& req, StatusCode &code) {
  if (req.headers.count("transfer-encoding") && req.headers.count("content-length")) {
    code = StatusBadRequest;
	return;
  }
  if (req.headers.count("transfer-encoding")) {
    if (req.transfer_encoding.back().token != "chunked") {
	  code = StatusBadRequest;
	  req.close = true;
      return;
    }
  } else if (req.headers.count("content-length") == 0) {
    req.content_length = 0;
  }
}

bool	add_body(Request &req, std::string const &data) {
  bool answer = read_body(req, data, 0, req.code);
  if (req.code != NoError) {
	req.representation.clear();
	return true;
  }
  return answer;
}

bool read_body(Request& req, std::string const &data, size_t begin, StatusCode &code) {
  bool answer = true;
  if (req.content_length == -1) {
	answer = decoding_chunked(req, data, begin, code);
	if (code != NoError)
	  return code;
  }
  else {
    uint64_t bytes_to_add = req.content_length;
    bytes_to_add -= req.body.length();
    if (bytes_to_add > data.length() - begin) {
      bytes_to_add = data.length() - begin;
      answer = false;
    }
	req.body.append(data, begin, bytes_to_add);
  }
  return answer;
}

/*
 * chunked-body = *chunk
 * last-chunk
 * trailer-part
 * CRLF
 *
 * chunk = chunk-size [ chunk-ext ] CRLF
 * chunk-data CRLF
 * last-chunk = 1*("0") [ chunk-ext ] CRLF
 *
 * chunk-data = 1*OCTET ; a sequence of chunk-size octets
 *
 * trailer-part = *( header-field CRLF )
 */
bool decoding_chunked(Request& req, std::string const &data, size_t begin, StatusCode &code) {
  size_t chunk_size;
  size_t pos = begin;
  // read chunk-size, chunk-ext(if any), and CRLF
  pos += read_chunk_size(chunk_size, data, pos, code);
  if (code != NoError)
	return code;

  //read chunk-data
  while (chunk_size > 0) {
    //read chunk-data and CRLF
    if (data.length() - pos < chunk_size) {
	  return code = StatusBadRequest;
    }
    if (req.body.length() + chunk_size > req.serv_config.limit_size) {
      req.close = true;
      req.code = StatusRequestEntityTooLarge;
	  return true;
//      req.body.append(data, pos, req.serv_config.limit_size - req.body.length());
    } else {
	  req.body.append(data, pos, chunk_size);
    }
    pos += chunk_size;
    if (data.compare(pos, 2, "\r\n") != 0) {
	  return code = StatusBadRequest;
    }
    pos += 2;
    if (data.length() - pos == 0)
	  return false;
    pos += read_chunk_size(chunk_size, data, pos, code);
    if (code != NoError)
	  return code;
  }
  //read trailer field
  std::map<std::string, std::string> trailer_headers;
  parse_headers(trailer_headers, data, pos, code);
  return true;
}

/*
 * chunk-size = 1*HEXDIG
 *
 * chunk-ext = *( ";" chunk-ext-name [ "=" chunk-ext-val ] )
 * chunk-ext-name = token
 * chunk-ext-val = token / quoted-string
 */
size_t read_chunk_size(size_t& chunk_size, std::string const &data, size_t begin,
					   StatusCode &code) {
  size_t pos = begin;
  std::string ext_name;
  std::string ext_val;

  //read chunk-size
  while (isxdigit(data[pos]))
	++pos;
  if (pos - begin == 0) {
	return code = StatusBadRequest;
  }
  std::stringstream ss(std::string(data, begin, pos - begin));
  ss >> std::hex >> chunk_size;
  if (ss.fail()) {
	return code = StatusBadRequest;
  }
  // chunk-ext(if any)
  while (data[pos] == ';') {
	++pos;
	pos += get_token(ext_name, data, pos, code);
	if (code != NoError)
	  return code;
	if (data[pos] == '=') {
	  ++pos;
	  if (data[pos] == DQUOTE) {
		++pos;
		pos += get_quoted_string(ext_val, data, pos, code);
		if (code != NoError)
		  return code;
	  } else {
		pos += get_token(ext_name, data, pos, code);
		if (code != NoError)
		  return code;
	  }
	}
	ext_name.clear();
	ext_val.clear();
  }
  pos += skip_crlf(data, pos, code);
  if (code != NoError)
	return code;
  return pos - begin;
}

std::string methodToString(Method m) {
  if (m == GET) {
	return "GET";
  } else if (m == POST) {
	return "POST";
  } else if (m == DELETE) {
	return "DELETE";
  }
  return "";
}

std::string get_random_str() {
  std::string answer;
  answer.reserve(8);
  const std::string ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
							   "abcdefghijklmnopqrstuvwxyz"
							   "0123456789";
  srand(std::time(NULL));
  for (int i = 0; i < 8; ++i) {
    answer += ALPHABET[rand() % ALPHABET.length()];
  }
  return answer;
}

std::string get_random_filename(const std::string &name_dir) {
  std::string answer;
  struct stat buf = {};
  do {
    std::string random_str = get_random_str();
    answer = name_dir + random_str;
  } while (stat(answer.c_str(), &buf) != -1);
  return answer;
}

bool	check_config(Request& req) {
  if (req.content_length != -1) {
    if (size_t(req.content_length) > req.serv_config.limit_size) {
      req.content_length = static_cast<int64_t>(req.serv_config.limit_size);
    }
  }
  if (req.serv_config.accepted_methods.count(methodToString(req.method)) == 0) {
    req.code = StatusMethodNotAllowed;
	return false;
  }
  if (!req.serv_config.redirection_url.empty()) {
    req.code = StatusCode(req.serv_config.redirection_status_code);
	return false;
  }
  struct stat buf = {};
  if (stat(req.serv_config.name_file.c_str(), &buf) == -1) {
	if (errno == ENOENT) {
	  req.code = StatusNotFound;
	} else if (errno == EACCES) {
	  req.code = StatusNotFound;
	} else {
	  req.code = StatusInternalServerError;
	}
	return false;
  }
  switch (req.method) {
	case GET:
	  if (S_ISDIR(buf.st_mode)) {
		if (req.serv_config.autoindex) {
		  req.representation = req.serv_config.name_file;
		} else {
		  req.representation = req.serv_config.file_request_if_dir;
		}
	  } else {
	    req.representation = req.serv_config.name_file;
	  }
	  req.code = StatusOK;
	  return true;
	case POST:
	  if (S_ISDIR(buf.st_mode)) {
	    if (!req.serv_config.route_for_uploaded_files.empty()) {
		  req.representation =
			  get_random_filename(req.serv_config.route_for_uploaded_files);
		  req.code = StatusCreated;
		} else {
		  req.representation = get_random_filename(req.serv_config.name_file);
		  req.code = StatusCreated;
		}
	  } else {
	    req.representation = req.serv_config.name_file;
	    req.code = StatusOK;
	  }
	  return true;
	case DELETE:
	  if (S_ISDIR(buf.st_mode)) {
	    req.code = StatusForbidden;
		return false;
	  } else {
	    req.representation = req.serv_config.name_file;
	    req.code = StatusOK;
		return true;
	  }
  }
  return true;
}

Response::Response() : code(NoError) {}

void get_response(const Request& req, Response &response) {
//  std::string answer;
//  if (req.code != NoError) {
//    answer.append("HTTP/1.1 400 BadRequest\r\n\r\n");
//  } else {
//	answer.append("HTTP/1.1 200 OK\r\n"
//				  "Content-Length: 12\r\n"
//				  "Content-Type: text/plain\r\n"
//				  "\r\n"
//				  "Hello world!");
//  }
//  return answer;
	if (response.code == NoError) {
	  response.code = req.code;
	}
	switch (response.code) {
	  case NoError:
	  case StatusOK: error200(req, response);
	  case StatusCreated: error201(req, response);
	  case StatusMovedPermanently: error301(req, response);
	  case StatusFound: error302(req, response);
	  case StatusSeeOther: error303(req, response);
	  case StatusTemporaryRedirect: error307(req, response);
	  case StatusBadRequest: error400(req, response);
	  case StatusForbidden: error403(req, response);
	  case StatusNotFound: error404(req, response);
	  case StatusMethodNotAllowed: error405(req, response);
	  case StatusRequestTimeout: error408(req, response);
	  case StatusRequestEntityTooLarge: error413(req, response);
	  case StatusRequestURITooLong: error414(req, response);
	  case StatusInternalServerError: error500(req, response);
	  case StatusNotImplemented: error501(req, response);
	  case StatusServiceUnavailable: error503(req, response);
	  case StatusHTTPVersionNotSupported: error505(req, response);
	}
}

void ResponseToString(const Response &resp, std::string &str) {
  str.append("HTTP/1.1 ");
  std::stringstream ss;
  ss << resp.code;
  std::string code;
  ss >> code;
  str.append(code);
  str.append(" " + resp.status + "\r\n");
  Headers::const_iterator begin = resp.header.begin();
  Headers::const_iterator end = resp.header.end();
  while (begin != end) {
    str.append((*begin).first + ": " + (*begin).second + "\r\n");
    ++begin;
  }
  str.append("\r\n");
  str.append(resp.body);
}

}