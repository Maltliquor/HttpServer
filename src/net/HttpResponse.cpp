#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "src/net/HttpResponse.h"

using namespace serverlib;


void HttpResponse::getResponseMsg(std::string& output) {
    output += "HTTP/1.1 "+ std::to_string(_status_code) + " ";
    output += _status_msg;
    output += "\r\n";

    if (_close_connection)  {
		output += "Connection: close\r\n";
    }
    else {
		output += "Content-Length: "+ std::to_string(_body.size())+ "\r\n";
		output += "Connection: Keep-Alive\r\n";
    }

	for (const auto& header : _headers) {
		output += header.first;
		output += ": ";
		output += header.second;
		output += "\r\n";
	}
	output += "\r\n";
    output += _body;
}
void HttpResponse::setStatusMessage(HttpStatus code){
	std::string message = "OK";
	
    switch (code){
        case OK200:
            message = "OK";
            break;
        case MovedPermanetly301:
            message = "Moved Permanetly";
            break;
        case BadRequest400:
            message = "Bad Request";
            break;
		case NotFound404:
            message = "Not Found";        
            break;
        default:
            message = "Internal Server Error";
            break;
    }
    _status_msg = message;

}
void HttpResponse::setContentTypeByUrl(const std::string& url_path){
	int pos = url_path.find_last_of('.');
	assert(pos!=-1);
	std::string file_extension = url_path.substr(pos);
    std::string type = "text/plain; charset=gbk";
	
	if(file_extension==".aspx" 
		|| file_extension==".html" 
		|| file_extension==".htm")  type = "text/html; charset=UTF-8";	
	if(file_extension==".png")    	type = "image/png";
	if(file_extension==".gif")    	type = "image/gif";
	if(file_extension==".jpg" 
		|| file_extension==".jpeg") type = "image/jpeg";
    if(file_extension==".css")    	type = "text/css";
	if(file_extension==".js")    	type = "application/x-javascript";
       
	addHeader("Content-Type", type);
}

bool HttpResponse::readURL(const std::string& file_dir, const std::string& url, std::string& out_buffer){
    int pos = url.find_last_of(".");
	std::string file_extension = url.substr(pos+1); //file_extension不包含“.”
	
    std::string path;
	//path = file_dir + "/" + file_extension + "/" + url;
	path = file_dir + "/"  + url;	
	
    struct stat stbuf;    
    
	/// 查看文件是否存在
    if(stat(path.c_str(),&stbuf)==-1){//文件不存在
		printf("Cannot find file or directory :%s \n", path.c_str());
        _status_code = NotFound404;
		return false;
    }else if(S_ISREG(stbuf.st_mode)){
        int src_fd = ::open(path.c_str(), O_RDONLY, 0);
		if (src_fd < 0) {
			printf("Cannot open file or directory :%s \n", path.c_str());
			out_buffer.clear();
			_status_code = NotFound404;
			return false;
		}
		void *mmapRet = mmap(NULL, stbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
		::close(src_fd);
		if (mmapRet == (void *)-1) {
			printf("Cannot mmap file or directory :%s \n", path.c_str());
			munmap(mmapRet, stbuf.st_size);
			out_buffer.clear();
			_status_code = NotFound404;
			return false;
		}
		
        char *src_addr = static_cast<char *>(mmapRet);
		out_buffer += std::string(src_addr, src_addr + stbuf.st_size);
		printf("Acessing file or directory :%s \n", path.c_str());
    }	
	
	return true;
} 
void HttpResponse::setHeader(){
	_headers["Http_version"] = "1.1";
    _headers["status_code"] = _status_code;
    _headers["content_type"] = _content_type;
}