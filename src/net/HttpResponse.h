#pragma once

#include <string>
#include <sys/types.h>
#include <map>
#include <assert.h>

namespace serverlib
{

class HttpResponse{
 public:
    enum HttpStatus{
            Unknown,
            OK200=200,
            MovedPermanetly301 = 301,
            BadRequest400 = 400,
            NotFound404 = 404,
    };

    explicit HttpResponse(): _status_code(Unknown), _close_connection(false){ }
    explicit HttpResponse(bool close): _status_code(Unknown), _close_connection(close){ }

    void setStatusCode(HttpStatus code)
	{ 
	    _status_code = code;
		setStatusMessage(code);
	}

    void setStatusMessage(HttpStatus code);
     //   { _status_msg = message; }

    void setCloseConnection(bool on)
        { _close_connection = on; }
		
	bool closeConnection() const
        {  return _close_connection; }

	void setContentTypeByUrl(const std::string& url_path);
	
    void setContentType(const std::string& content_type)
        { addHeader("Content-Type", content_type); }

	void setHeader();
    void addHeader(const std::string& key, const std::string& value)
        { _headers[key] = value; }
	const std::map<std::string, std::string>& headers() const
		{ return _headers; }
	
    void setBody(const std::string& body)
        { _body = body; }
	
	void getResponseMsg(std::string& output);
	bool readURL(const std::string& file_dir, 
				 const std::string& url, 
				 std::string& out_buffer);
	
 private:
	std::map<std::string, std::string> _headers;
	HttpStatus _status_code;
	std::string _content_type;
	std::string _status_msg;
	bool _close_connection;
	std::string _body;
};

}