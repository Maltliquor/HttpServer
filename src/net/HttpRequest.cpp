#include "src/net/HttpRequest.h"

using namespace serverlib;

/// Standard Http Header
///     GET/sample.Jsp HTTP/1.1[CRLF]               ＜request-line＞
///     Accept:image/gif.image/jpeg,*/*             ＜headers＞
///     Accept-Language:zh-cn
///     Connection:Keep-Alive
///     Host:localhost
///     User-Agent:Mozila/4.0(compatible;MSIE5.01;Window NT5.0)
///     Accept-Encoding:gzip,deflate[CRLF]			＜headers＞
///     [CRLF]                                      ＜blank line＞
///     username=jinqiao&password=1234             [＜request-body＞]

/// Standard Http Response
/// 	HTTP/1.1 200 OK[CRLF]                       ＜status-line＞
/// 	Date: Sat, 31 Dec 2005 23:59:59 GMT         ＜headers＞
/// 	Content-Type: text/html;charset=ISO-8859-1
/// 	Content-Length: 122[CRLF]					＜headers＞
/// 	[CRLF]										＜blank line＞
/// 	＜html＞								   [＜request-body＞]
/// 	＜head＞
/// 	＜title＞Homepage＜/title＞
/// 	＜/head＞
/// 	＜body＞
/// 	＜!-- body goes here --＞
/// 	＜/body＞
/// 	＜/html＞



std::string HttpRequest::getHeader(const std::string& field) const{
    std::string result;
    std::map<std::string, std::string>::const_iterator it = _headers.find(field);
    if (it != _headers.end())
    {
      result = it->second;
    }
    return result;
}

  
bool HttpRequest::setMethod(const char* start, const char* end){
	assert(_method == mINVALID);
	
	std::string m(start, end);
	if(m == "GET")	_method = mGET;
	else if(m == "POST")	_method = mPOST;
	else if(m == "HEAD")	_method = mHEAD;
	else if(m == "PUT")	_method = mPUT;
	else if(m == "DELETE")	_method = mDEL;
	else  _method = mINVALID;
	
	return _method != mINVALID;
}
	
const char* HttpRequest::methodString() const
{
	const char* result = "UNKNOWN";
	switch(_method)	{
		case mGET:
			result = "GET";
			break;
		case mPOST:
			result = "POST";
			break;
		case mHEAD:
			result = "HEAD";
			break;
		case mPUT:
			result = "PUT";
			break;
		case mDEL:
			result = "DELETE";
			break;
		default:
			break;
	}
	return result;
}

/// params:
///   start: start position of buf
///   colon: ":" position of buf
///   end:   end position of buf
void HttpRequest::addHeader(const char* start, const char* colon, const char* end){
    std::string field(start, colon);
	
    ++colon;
    while (colon < end && isspace(*colon)){//跳过冒号后面的 空格
        ++colon;
    }
    std::string value(colon, end);
    while (!value.empty() && isspace(value[value.size()-1]))  { //提取“：”后面的内容
        value.resize(value.size()-1);
    }
    _headers[field] = value;
}

bool HttpRequest::processRequestLine(const char* begin, const char* end){
	
    bool succeed = false;
    const char* start = begin;
    const char* space = std::find(start, end, ' ');
	std::string temp(start, end);
    if (space != end && setMethod(start, space)) { //提取方法/GET/HEAD/POST/...
		start = space+1;
		space = std::find(start, end, ' ');
		if (space != end) {
		    const char* question = std::find(start, space, '?');
		    if (question != space) {
				setPath(start, question);   //提取URL
				setQuery(question, space);  //提取query
		    }
		    else  {
				setPath(start, space);      //提取URL
		    }
		    start = space+1;
		    succeed = end-start == 8 && std::equal(start, end-1, "HTTP/1.");
		    if (succeed) {
				if (*(end-1) == '1'){
					setVersion(HttpRequest::vHttp11);   //提取Http协议版本
				}
				else if (*(end-1) == '0'){
					setVersion(HttpRequest::vHttp10);
				}
				else{
					succeed = false;
				}
		    }
		}
    }
    return succeed;
}

// return false if any error
bool HttpRequest::parseRequest(const std::vector<char>& buf, int len){	
	//FIXME:std::string sbuf(buf);
	//printf("HttpRequest::parseRequest() parsing sbuffer:\n%s\n", buf);
	std::string sbuf;
	sbuf.clear();
	sbuf.assign(buf.begin(),buf.end());
	
	std::string CRLF = "\r\n";
	bool ok = true;
	bool has_more = true;
	int begin = 0;
	size_t pos = 0;
	
	//printf("HttpRequest::parseRequest() parsing sbuffer:\n%s\n", sbuf.c_str());
	while (has_more){
		if (_state == kExpectRequestLine){
			pos = sbuf.find_first_of(CRLF,begin);
			//const char* crlf = buf->findCRLF();
			
			//FIXME
			if(pos != std::string::npos){
				ok = processRequestLine(&*(sbuf.begin()+begin), &*(sbuf.begin()+pos));
				//printf("%d %s %s", _method,_path.c_str(),_query.c_str());
				if (ok) {
					//buf->retrieveUntil(crlf + 2);
					//sbuf = sbuf.substr(begin, pos - begin);
					_state = kExpectHeaders;
					begin = pos + CRLF.size();
				}
				else {
					has_more = false;
				}
			}
			else {
				has_more = false;
			}
		}
		else if (_state == kExpectHeaders) {
		    pos = sbuf.find_first_of(CRLF, begin);
			//printf("pos:%d substr:%s\n", pos, sbuf.substr(begin, pos-begin).c_str());
			//const char* crlf = buf->findCRLF();
		    if (pos != std::string::npos) {
				const char* colon = std::find(&*(sbuf.begin()+begin), &*(sbuf.begin()+pos), ':');
				//size_t colon = sbuf.find(":",begin, pos-begin);
				//printf("begin:%d colon:%d\n", begin, colon == &*(sbuf.begin()+pos));
				if (colon != (&*(sbuf.begin()+pos))) {
				    //addHeader(&*(sbuf.begin()+begin), &*(sbuf.begin()+colon), &*(sbuf.begin()+pos));
					addHeader(&*(sbuf.begin()+begin), colon, &*(sbuf.begin()+pos));
				}
				else {
				    // empty line, end of header
				    _state = kGotAll;
				    has_more = false;
				}
			    begin = pos + CRLF.size();
		    }
		    else {
				has_more = false;
		    }
		}
		else if (_state == kExpectBody)  {
		  // 待完成，实现POST等功能
		  // 解析requestBody部分
		}
	}
	
	return ok;
}
