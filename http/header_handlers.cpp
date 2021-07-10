#include "http.hpp"
#include "utility_http.hpp"
#include "sstream"

namespace http {

const char	DQUOTE = '\"';

HeaderHandlers &get_header_field_handlers() {
  static HeaderHandlers header_field_handlers = {
	  {"content-length", &content_length},
	  {"transfer-encoding", &transfer_encoding},
	  {"host", &host},
	  {"content-type", &content_type},
	  {"content-language", &content_language},
	  {"content-location", &content_location},
  };
  return header_field_handlers;
}

TransferCodingRegister& get_transfer_coding_register() {
  static const TransferCodingRegister transfer_coding_register = {
	  {"chunked", 1},
  };
  return transfer_coding_register;
}

MediaTypeRegister& get_media_type_register() {
  static const MediaTypeRegister type_register = {
	  {"application/http", 1},
	  {"message/http", 1},
  };
  return type_register;
}
void header_analysis(Request &req, Headers &h, StatusCode &err) {
  Headers::const_iterator first;
  Headers::const_iterator last;
  first = h.begin();
  last = h.end();

  HeaderHandlers &handlers = get_header_field_handlers();
  if (handlers.count("host") == 0) {
    err = StatusBadRequest;
	return;
  }
  for (; first != last; ++first) {
	if (handlers.count(first->first))
	  handlers.at(first->first)(req, first->second, err);
	if (err != 0)
	  return;
  }
}

// Content-Length = 1*DIGIT
void content_length(Request& req, std::string const &value, StatusCode &err) {
  if (value.find_first_not_of("0123456789") != std::string::npos) {
	err = StatusBadRequest;
	return;
  }
  std::stringstream ss(value);
  ss >> req.content_length;
  if (ss.fail()) {
    err = StatusBadRequest;
	return;
  }
  if (req.content_length < 0) {
    err = StatusBadRequest;
	return;
  }
}

void validate_transfer_coding(const std::string& name, StatusCode &err) {
  TransferCodingRegister& r = get_transfer_coding_register();
  if (r.count(name) == 0)
	err = StatusNotImplemented;
}

// transfer-extension = token *( OWS ";" OWS transfer-parameter )
void transfer_encoding(Request& req, std::string const &extension, StatusCode &err) {
  size_t begin_word = 0;
  size_t end_word = 0;
  transfer_extension tmp_ext;
  parameter tmp_par;

  while (begin_word != extension.size()) {
	end_word += get_token(tmp_ext.token, extension, begin_word, err);
	if (err != 0)
	  return;
	tolower(tmp_ext.token);
	validate_transfer_coding(tmp_ext.token, err);
	if (err != 0)
	  return;
	end_word += skip_space(extension, end_word, OWS, err);
	if (err != 0)
	  return;
	while (extension[end_word] == ';') {
	  ++end_word;
	  end_word += skip_space(extension, end_word, OWS, err);
	  if (err != 0)
		return;
	  // transfer-parameter = token BWS "=" BWS ( token / quoted-string )
	  begin_word = end_word;
	  end_word += get_token(tmp_par.name, extension, begin_word, err);
	  if (err != 0)
		return;
	  end_word += skip_space(extension, end_word, BWS, err);
	  if (err != 0)
		return;
	  if (extension[end_word] != '=') {
	    err = StatusBadRequest;
		return;
	  }
	  ++end_word;
	  end_word += skip_space(extension, end_word, BWS, err);
	  if (err != 0)
		return;
	  begin_word = end_word;
	  //quoted-string / token
	  if (extension[end_word] == DQUOTE) {
		++end_word;
		++begin_word;
		end_word += get_quoted_string(tmp_par.value, extension, begin_word, err);
		if (err != 0)
		  return;
	  } else {
		end_word += get_token(tmp_par.value, extension, begin_word, err);
		if (err != 0)
		  return;
	  }
	  tmp_ext.transfer_parameter.push_back(tmp_par);
	  tmp_par.name.clear();
	  tmp_par.value.clear();
	  end_word += skip_space(extension, end_word, OWS, err);
	  if (err != 0)
		return;
	}
	req.transfer_encoding.push_back(tmp_ext);
	while (extension[end_word] == ',' || isblank(extension[end_word]))
	  ++end_word;
	begin_word = end_word;
	tmp_ext.token.clear();
	tmp_ext.transfer_parameter.clear();
  }
}

// Host = uri-host [":" port]
void host(Request& req, std::string const &value, StatusCode &err) {
  size_t host = 0;
  host += url::get_host(req.url.host, value, host);
  size_t port = host + 1;
  if (host != value.length()) {
	if (value[host] == ':') {
	  req.url.host += ":";
	  port += url::get_port(req.url.host, value, port);
	  if (port != value.length()) {
	    err = StatusBadRequest;
		return;
	  }
	} else {
	  err = StatusBadRequest;
	}
  }
}

void validate_media_type(const media_type& type, StatusCode &err) {
  MediaTypeRegister& type_register = get_media_type_register();
  if (type_register.count(type.type + "/" + type.subtype) == 0) {
	err = StatusNotImplemented;
	return;
  }
//  for (size_t i = 0; i < type.parameters.size(); ++i) {
//    parameter tmp_par = type.parameters[i];
//	tolower(tmp_par.value);
//	tolower(tmp_par.name);
//    if (tmp_par.name == "version") {
//      if (tmp_par.value != "1.1")
//		error1(505);
//	  continue;
//    }
//    if (tmp_par.name == "msgtype") {
//      if (tmp_par.value == "response" || tmp_par.value == "request") {
//		continue;
//      }
//    }
//	error1(StatusBadRequest);
//  }
}

/*
 * media-type = type "/" subtype *( OWS ";" OWS parameter )
 * type = token
 * subtype = token
 */
void content_type(Request &req, std::string const &value, StatusCode &err) {
  size_t begin_word = 0;
  parameter tmp_par;

  begin_word += get_token(req.metadata.media_type.type, value, begin_word, err);
  if (err != 0)
	return;
  tolower(req.metadata.media_type.type);
  if (value[begin_word] != '/') {
    err = StatusBadRequest;
	return;
  }
  ++begin_word;
  begin_word += get_token(req.metadata.media_type.subtype, value, begin_word, err);
  if (err != 0)
	return;
  tolower(req.metadata.media_type.subtype);
//  validate_media_type(req.metadata.media_type);

  begin_word += skip_space(value, begin_word, OWS, err);
  if (err != 0)
	return;

  while (value[begin_word] == ';') {
    ++begin_word;
    begin_word += skip_space(value, begin_word, OWS, err);
	if (err != 0)
	  return;
	// parameter = token "=" ( token / quoted-string )
    begin_word += get_token(tmp_par.name, value, begin_word, err);
	if (err != 0)
	  return;
	if (value[begin_word] != '=') {
	  err = StatusBadRequest;
	  return;
	}
    ++begin_word;
    //quoted-string / token
    if (value[begin_word] == DQUOTE) {
      ++begin_word;
      begin_word += get_quoted_string(tmp_par.value, value, begin_word, err);
	  if (err != 0)
		return;
	} else {
      begin_word += get_token(tmp_par.value, value, begin_word, err);
	  if (err != 0)
		return;
	}
	  req.metadata.media_type.parameters.push_back(tmp_par);
	  tmp_par.name.clear();
	  tmp_par.value.clear();
	  begin_word += skip_space(value, begin_word, OWS, err);
	if (err != 0)
	  return;
  }
}

// Content-Language = 1#language-tag
void content_language(Request &req, std::string const &value, StatusCode &err) {
  size_t begin_world = 0;
  size_t end_world;
  while (value[begin_world] == ',') {
    ++begin_world;
    begin_world += skip_space(value, begin_world, OWS, err);
	if (err != 0)
	  return;
  }
  end_world = begin_world;
  while (end_world < value.length() && isalnum(value[end_world])) {
    ++end_world;
  }
  if (end_world - begin_world == 0) {
    err = StatusBadRequest;
	return;
  }
  req.metadata.content_language.push_back(value.substr(begin_world, end_world -
  begin_world));
  begin_world = end_world;
  while (begin_world < value.length()) {
    begin_world += skip_space(value, begin_world, OWS, err);
	if (err != 0)
	  return;
	if (value[begin_world] != ',') {
	  err = StatusBadRequest;
	  return;
	}
    ++begin_world;
	begin_world += skip_space(value, begin_world, OWS, err);
	if (err != 0)
	  return;
	end_world = begin_world;
	while (end_world < value.length() && isalnum(value[end_world])) {
	  ++end_world;
	}
	if (end_world - begin_world != 0) {
	  req.metadata.content_language.push_back(value.substr(begin_world, end_world -
		  begin_world));
	}
	begin_world = end_world;
  }
}

// Content-Location = absolute-URI / partial-URI
void content_location(Request &req, std::string const &value, StatusCode &) {
  if (value.length() != 0 && value[0] == '/') {
    parse_partial_uri(req.url, value);
  } else {
    parse_absolute_uri(req.url, value);
  }
}

}