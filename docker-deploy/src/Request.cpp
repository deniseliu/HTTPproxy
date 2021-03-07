#include "Request.h"
#include<iostream>
Request::Request(std::string _message){
    message = _message;
    startLine=setLine(_message);
    splitLine = split(startLine,' ');
  }

std::vector<std::string> Request::split(std::string line, char delimiter){
    line += delimiter;
    std::vector<std::string> res;
    int n = line.length();
    std::string temp="";
    for(int i=0;i<n;i++){
      if(line[i]==delimiter){
        res.push_back(temp);
        temp="";
        
      }else{
        temp+=line[i];
      }
    }
    return res;
  }

std::string Request::setLine(std::string info){
    size_t firstLineEnd = info.find("\r\n");
    return info.substr(0,firstLineEnd);
  }

std::string Request::getMethod(){
    return splitLine[0];
  }

std::string Request::getURI(){
    return splitLine[1];
  }

std::string Request::getHost(){
    std::string uri = splitLine[1];
    size_t position=uri.find("http://");
    int start = 0;
    if(position==0){
      start = 7;
    }
    size_t nextSlash = uri.find("/",start);
    if(nextSlash==std::string::npos){
      nextSlash=uri.length();
    }
    std::string res = uri.substr(start,nextSlash-start);
    size_t colon = res.find(":");
    if(colon!=std::string::npos){
      res = res.substr(0,colon);
    }
    return res;
  }
std::string Request::getPort(){
    std::string uri = splitLine[1];
    size_t position=uri.find("http://");
    int start = 0;
    std::string port="";
    if(position==0){
      start = 7;
    }
    size_t slash = uri.find("/",start);
    size_t colon =uri.find(":",start);
    if(slash>colon){
    if(colon!=std::string::npos){
      size_t nextSlash = uri.find("/",colon);
      if(nextSlash==std::string::npos){
        nextSlash=uri.length();
      }
      port= uri.substr(colon+1,nextSlash-colon-1);
    }
    }
    if(port==""){
      return "80";
    }else{
      return port;
    }
  }

bool Request::checkFormat(){
    if(splitLine.size()!=3){
      return false;
    }
    std::string method = getMethod();
    if(method!="CONNECT"&&method!="GET"&&method!="POST"){
      return false;
    }
    return true;
  }
std::string Request::getStartLine(){
  return startLine;
}
std::string Request::getHeader(){
  int start = message.find("\r\n\r\n");
  return message.substr(0,start);
}
