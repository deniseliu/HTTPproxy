#include "Response.h"

Response::Response(std::vector<std::vector<char>> message){
  this->message = message;
  //std::cout<<message;
  if(this->message.size()!=0){
    this->headerPart = std::string(message[0].begin(),message[0].end());
    startLine=getStartLine();
    splitLine = split(startLine,' ');
    setupCacheField();
  }
  
}
std::vector<std::string> Response::split(std::string line, char delimiter){
    line += delimiter;
    std::vector<std::string> res;
    int n = line.length();
    std::string temp="";
    int index = 0;
    for(int i=0;i<n;i++){
      if(line[i]==delimiter){
        res.push_back(temp);
        temp="";
        index++;
      
      }else{
        temp+=line[i];
      }
    } 
    return res;
  }

bool Response::checkFormat(){
  if(splitLine.size()!=3&&splitLine.size()!=4){
      return false;
    }
  std::cout<<"size is :"<<splitLine.size()<<"\n";
  return true;
}

std::string Response::getStartLine(){
    size_t firstLineEnd = headerPart.find("\r\n");
    return headerPart.substr(0,firstLineEnd);
}

std::string Response::getField(std::string header){
  if(headerPart.find(header)!=std::string::npos){
    size_t start = headerPart.find(header);
    size_t end=headerPart.find("\r\n",start);
    return headerPart.substr(start+header.length(),end-start-header.length());
  }else{
    return "";
  }
}
void Response::setupCacheField(){
  std::string cacheControlHeader=getField("Cache-Control:");

  if(cacheControlHeader!=""){
    std::vector<std::string> field = split(cacheControlHeader,',');
    size_t size = field.size();
    for(size_t i=0;i<size;i++){   
      std::string value = getValue(field[i]);
      if(value!=""){
        std::string key = field[i].substr(1,field[i].length()-value.length()-2);
        cacheField[key]=value;
      }else{
        std::string key = field[i].substr(1);
        cacheField[key]="true";
      }
    }
  }
}

std::string Response::getValue(std::string s){
  if(s.find("=")!=std::string::npos){
    int start = s.find("=");
    return s.substr(start+1);
  }else{
    return "";
  }
}

std::map<std::string,std::string> Response::getCacheField(){
  return cacheField;
}
std::vector<std::vector<char>> Response::getMessage(){
    return message;
}
std::string Response::getEtag(){
  return getField("Etag: ");
}
std::string Response::getExpires(){
  return getField("Expires: ");
}
std::string Response::getLastModified(){
  return getField("Last-Modified: "); 
}

std::string Response::getDate(){
  return getField("Date: "); 
}

std::string Response::makeEtagRequest(Request r){
  std::string header = r.getHeader();
  std::string etag = getEtag();
  std::string newRequest = header+"\r\n"+"If-None-Match: "+etag+"\r\n\r\n";
  return newRequest;  
}
std::string Response::makeModifiedRequest(Request r){
  std::string header = r.getHeader();
  std::string last_modified = getLastModified();
  std::string newRequest = header+"\r\n"+"If-Modified-Since: "+last_modified+"\r\n\r\n";
  return newRequest;  
}
int Response::getHeaderSize(){
  int start = headerPart.find("\r\n\r\n");
  //std::cout<<"body is: \n"<<message.substr(s)<<"\n";
  return start+sizeof("\r\n\r\n");
}

bool Response::checkChunkSize(std::string body){
  int start = 0;
  while(1){
    int end_1 = body.find("\r\n",start);
    std::string chunk_size = body.substr(start,end_1);
    long _size = strtol(chunk_size.c_str(),NULL,16);
    int new_start = end_1+2;
    if(_size==0){
      if(body.substr(new_start)!="\r\n"){
        return false;
      }else{
        return true;
      }
    }
    int end_2 = body.find("\r\n",new_start);
    if((end_2-new_start)!=_size){
      std::cout<<"chunk body size doesn't fit\n";
      return false;
    }
    start = end_2+2;
  }
}

size_t Response::getContentLength(){
  std::string len = getField("Content-Length: ");
  if(len==""){
    return 0;
  }
  size_t contlen = strtol(len.c_str(),NULL,10);
  return contlen;
}
bool Response::checkContentLength(std::string body){
  std::string len = getField("Content-Length: ");
  long contlen = strtol(len.c_str(),NULL,10);
  if(body.length()!=contlen){
    return false;
  }else{
    std::cout<<"Equal!\n";
    return true;
  }
}

bool Response::notCorrupted(){
  if(!checkFormat()){
    return false;
  }
  std::string body;
  if(getField("Transfer-Encoding: ")=="chunked"){
    return checkChunkSize(body);
  }
  if(getField("Content-Length: ")!=""){
    return checkContentLength(body);
  }
  return false;
}
