#include "errors.hpp"
#include <sstream>
#include <fstream>

namespace http {

const char *const BadRequest = "<html>\n"
							   "<head><title>400 Bad Request</title></head>\n"
							   "<body>\n"
							   "<center><h1>400 Bad Request</h1></center>\n"
							   "</body>\n"
							   "</html>";

const char *const Forbidden = "<html>\n"
							  "<head><title>403 Forbidden</title></head>\n"
							  "<body>\n"
							  "<center><h1>403 Forbidden</h1></center>\n"
							  "</body>\n"
							  "</html>";

const char *const NotFound = "<html>\n"
							 "<head><title>404 Not Found</title></head>\n"
							 "<body>\n"
							 "<center><h1>404 Not Found</h1></center>\n"
							 "</body>\n"
							 "</html>";

const char *const MethodNotAllowed = "<html>\n"
									 "<head><title>405 Method Not Allowed</title></head>\n"
									 "<body>\n"
									 "<center><h1>405 Method Not Allowed</h1></center>\n"
									 "</body>\n"
									 "</html>";

const char *const RequestTimeout = "<html>\n"
								   "<head><title>408 Request Timeout</title></head>\n"
								   "<body>\n"
								   "<center><h1>408 Request Timeout</h1></center>\n"
								   "</body>\n"
								   "</html>";

const char *const RequestEntityTooLarge = "<html>\n"
										  "<head><title>413 Request Entity Too "
										  "Large</title></head>\n"
										  "<body>\n"
										  "<center><h1>413 Request Entity Too "
										  "Large</h1></center>\n"
										  "</body>\n"
										  "</html>";

const char *const RequestURITooLong = "<html>\n"
									  "<head><title>414 Request URI Too Long"
									  "</title></head>\n"
									  "<body>\n"
									  "<center><h1>414 Request URI Too Long"
									  "</h1></center>\n"
									  "</body>\n"
									  "</html>";

const char *const InternalServerError = "<html>\n"
										"<head><title>500 Internal Server Error"
										"</title></head>\n"
										"<body>\n"
										"<center><h1>500 Internal Server Error"
										"</h1></center>\n"
										"</body>\n"
										"</html>";

const char *const NotImplemented = "<html>\n"
								   "<head><title>501 Not Implemented"
								   "</title></head>\n"
								   "<body>\n"
								   "<center><h1>501 Not Implemented"
								   "</h1></center>\n"
								   "</body>\n"
								   "</html>";

const char *const ServiceUnavailable = "<html>\n"
									   "<head><title>503 Service Unavailable"
									   "</title></head>\n"
									   "<body>\n"
									   "<center><h1>503 Service Unavailable"
									   "</h1></center>\n"
									   "</body>\n"
									   "</html>";

const char *const HTTPVersionNotSupported = "<html>\n"
											"<head><title>505 HTTP Version Not Supported"
											"</title></head>\n"
											"<body>\n"
											"<center><h1>505 HTTP Version Not Supported"
											"</h1></center>\n"
											"</body>\n"
											"</html>";

void error200(const Request &, Response &resp) {
  resp.status = "OK";
  resp.code = StatusOK;
}

void error201(const Request &, Response &resp) {
  resp.status = "Created";
  resp.code = StatusCreated;
}

void error301(const Request &req, Response &resp) {
  resp.status = "Moved Permanently";
  resp.code = StatusMovedPermanently;
  resp.header["Location"] = req.serv_config.redirection_url;
  resp.header["Content-length"] = "0";
}

void error302(const Request &req, Response &resp) {
  resp.status = "Found";
  resp.code = StatusFound;
  resp.header["Location"] = req.serv_config.redirection_url;
  resp.header["Content-length"] = "0";
}

void error303(const Request &req, Response &resp) {
  resp.status = "See Other";
  resp.code = StatusSeeOther;
  resp.header["Location"] = req.serv_config.redirection_url;
  resp.header["Content-length"] = "0";
}

void error307(const Request &req, Response &resp) {
  resp.status = "Temporary Redirect";
  resp.code = StatusTemporaryRedirect;
  resp.header["Location"] = req.serv_config.redirection_url;
  resp.header["Content-length"] = "0";
}

bool read_all_file(const std::string &name, std::string &body) {
  std::ifstream in(name.c_str(), std::ios::in | std::ios::binary);
  if (in) {
	in.seekg(0, std::ios::end);
	body.resize(in.tellg());
	in.seekg(0, std::ios::beg);
	in.read(&body[0], static_cast<int64_t>(body.size()));
	in.close();
	if (in.bad() || in.fail()) {
	  body.clear();
	  return false;
	}
	return true;
  } else {
	return false;
  }
}

void error400(const Request &req, Response &resp) {
  resp.status = "Bad Request";
  resp.code = StatusBadRequest;
  if (req.serv_config.error_pages.count(400)) {
	if (!read_all_file(req.serv_config.error_pages.at(400), resp.body)) {
	  error500(req, resp);
	  return;
	}
  } else {
	resp.body = BadRequest;
  }
}

void error403(const Request &req, Response &resp) {
  resp.status = "Forbidden";
  resp.code = StatusForbidden;
  if (req.serv_config.error_pages.count(403)) {
	if (!read_all_file(req.serv_config.error_pages.at(403), resp.body)) {
	  error500(req, resp);
	  return;
	}
  } else {
	resp.body = Forbidden;
  }
}

void error404(const Request &req, Response &resp) {
  resp.status = "Not Found";
  resp.code = StatusNotFound;
  if (req.serv_config.error_pages.count(404)) {
	if (!read_all_file(req.serv_config.error_pages.at(404), resp.body)) {
	  error500(req, resp);
	  return;
	}
  } else {
	resp.body = NotFound;
  }
}

void error405(const Request &req, Response &resp) {
  resp.status = "Method Not Allowed";
  resp.code = StatusMethodNotAllowed;
  if (req.serv_config.error_pages.count(405)) {
	if (!read_all_file(req.serv_config.error_pages.at(405), resp.body)) {
	  error500(req, resp);
	  return;
	}
  } else {
	resp.body = MethodNotAllowed;
  }
}

void error408(const Request &req, Response &resp) {
  resp.status = "Request Timeout";
  resp.code = StatusRequestTimeout;
  if (req.serv_config.error_pages.count(408)) {
	if (!read_all_file(req.serv_config.error_pages.at(408), resp.body)) {
	  error500(req, resp);
	  return;
	}
  } else {
	resp.body = RequestTimeout;
  }
}

void error413(const Request &req, Response &resp) {
  resp.status = "Request Entity Too Large";
  resp.code = StatusRequestEntityTooLarge;
  if (req.serv_config.error_pages.count(413)) {
	if (!read_all_file(req.serv_config.error_pages.at(413), resp.body)) {
	  error500(req, resp);
	  return;
	}
  } else {
	resp.body = RequestEntityTooLarge;
  }
}

void error414(const Request &req, Response &resp) {
  resp.status = "Request URI Too Long";
  resp.code = StatusRequestURITooLong;
  if (req.serv_config.error_pages.count(414)) {
	if (!read_all_file(req.serv_config.error_pages.at(414), resp.body)) {
	  error500(req, resp);
	  return;
	}
  } else {
	resp.body = RequestURITooLong;
  }
}

void error500(const Request &req, Response &resp) {
  resp.status = "Internal Server Error";
  resp.code = StatusInternalServerError;
  if (req.serv_config.error_pages.count(500)) {
	if (!read_all_file(req.serv_config.error_pages.at(501), resp.body)) {
	  resp.body = InternalServerError;
	}
  } else {
	resp.body = InternalServerError;
  }
}

void error501(const Request &req, Response &resp) {
  resp.status = "Not Implemented";
  resp.code = StatusNotImplemented;
  if (req.serv_config.error_pages.count(501)) {
	if (!read_all_file(req.serv_config.error_pages.at(501), resp.body)) {
	  error500(req, resp);
	  return;
	}
  } else {
	resp.body = NotImplemented;
  }
}

void error503(const Request &req, Response &resp) {
  resp.status = "Service Unavailable";
  resp.code = StatusServiceUnavailable;
  if (req.serv_config.error_pages.count(503)) {
	if (!read_all_file(req.serv_config.error_pages.at(503), resp.body)) {
	  error500(req, resp);
	  return;
	}
  } else {
	resp.body = ServiceUnavailable;
  }
}

void error505(const Request &req, Response &resp) {
  resp.status = "HTTP Version Not Supported";
  resp.code = StatusHTTPVersionNotSupported;
  if (req.serv_config.error_pages.count(505)) {
	if (!read_all_file(req.serv_config.error_pages.at(505), resp.body)) {
	  error500(req, resp);
	  return;
	}
  } else {
	resp.body = HTTPVersionNotSupported;
  }
  }

void add_length(Response &resp, bool chunked) {
  if (chunked) {
    resp.header["Transfer-Encoding"] = "chunked";
  } else {
	std::stringstream ss;
	ss << resp.body.length();
	std::string length;
	ss >> length;
	resp.header["Content-length"] = length;
  }
}
}
