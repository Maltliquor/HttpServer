#pragma once

#include <map>
#include <assert.h>
#include <stdio.h>
#include <string>
#include <algorithm>
#include <vector>

namespace serverlib
{

class HttpRequest
{
	
 public:
	enum Method	{
		mINVALID, 
		mGET, 
		mPOST, 
		mHEAD, 
		mPUT, 
		mDEL,
	};
	enum Version{
		vUnknown, 
		vHttp10, 
		vHttp11
	};
	
	enum HttpRequestParseState{
		kExpectRequestLine,
		kExpectHeaders,
		kExpectBody,
		kGotAll,
    };
  
	HttpRequest() : _method(mINVALID), _version(vUnknown)  {}

	void setVersion(Version v) 
	{	_version = v;	}

	Version getVersion() const
	{ return _version; }

    bool setMethod(const char* start, const char* end);

    Method method() const
    { return _method; }

    const char* methodString() const;

	void setPath(const char* start, const char* end)
	{  _path.assign(start, end);  }

	const std::string& path() const
	{ return _path; }

	void setQuery(const char* start, const char* end)
	{ _query.assign(start, end);	}

	const std::string& query() const
	{ return _query; }


    void addHeader(const char* start, const char* colon, const char* end);

    std::string getHeader(const std::string& field) const;

	const std::map<std::string, std::string>& headers() const
	{ return _headers; }

    void swap(HttpRequest& that){
		std::swap(_method, that._method);
		std::swap(_version, that._version);
		_path.swap(that._path);
		_query.swap(that._query);
		_headers.swap(that._headers);
    }
	
	void reset() {
		_state = kExpectRequestLine;
		
		HttpRequest dummy;
		std::swap(_method, dummy._method);
		std::swap(_version, dummy._version);
		_path.swap(dummy._path);
		_query.swap(dummy._query);
		_headers.swap(dummy._headers);
	}
	// HttpContext return false if any error
	bool parseRequest(const std::vector<char>& buf, int len);

	bool isGotAll() const
	{ return _state == kGotAll; }


 private:
	bool processRequestLine(const char* begin, const char* end);

	HttpRequestParseState _state;
  
	Method _method;
	Version _version;
	std::string _path;
	std::string _query;
	std::map<std::string, std::string> _headers;

 
};

}  //namespace serverlib