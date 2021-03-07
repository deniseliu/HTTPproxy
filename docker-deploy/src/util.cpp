#include "Response.h"
#include <cstring>
#include <unordered_map>
std::unordered_map<std::string,Response> Cache;

bool checkCacheble(Response s){
  std::map<std::string,std::string> cacheField = s.getCacheField();
  if(cacheField.find("no-store")!=cacheField.end()){
    std::cout<<"not cacheble because no-store\n";
    return false;
  }
  if(cacheField.find("private")!=cacheField.end()){
    std::cout<<"not cacheble because private\n";
    return false;
  }
  return true;
}


std::string ask_reply(std::string req,int server_fd){
  int send_len;
  char req_msg[req.length()+1]={0};
  strcpy(req_msg,req.c_str());
  if((send_len=send(server_fd,req_msg,req.length()+1,0))>0){
    std::cout<<"request server for response\n"; //send server request
  }
   char new_res[65536] = {0};
   int new_len = recv(server_fd,&new_res,sizeof(new_res),0);
   /*
  std::vector<char> new_res(1000,0); 
  int start = 0;
  int new_len=0; 
  while(1){  // receive message in loop
    if((new_len=recv(server_fd,&new_res.data()[start],new_res.size(),0))<=0){
      break;
    }
    start += new_len;
    new_res.resize(2*new_res.size());   
  }
  std::string res(new_res.data(),start+new_len); //get response back
  */
  std::string res(new_res,new_len); //get response back 
  Response response(res);
  if(!response.notCorrupted()){ //if corrupted, send back 502 gateway
    std::cout<<"Corrupted response\n";
    res = "HTTP/1.1 502 Bad Gateway\r\n\r\n";
  }
  return res;
  
}
void saveRes(std::string res, std::string uri){
  if(res.find("HTTP/1.1 200 OK")!=std::string::npos){//cache response that is 200 ok
    Response s(res);
    if(checkCacheble(s)){
      Cache[uri]=s;
    }
  } 
}

std::string revalidate(int server_fd,Response s, std::string uri){
  std::string req;
  if(s.getLastModified()!=""){
    req = s.makeModifiedRequest(uri);      
  }
  if(s.getEtag()!=""){
    req = s.makeEtagRequest(uri);
  }
  std::string res = ask_reply(req,server_fd);//ask server if the response can still be used
  saveRes(res,uri); //get the response from server

  if(res.find("HTTP/1.1 304")!=std::string::npos){
    return Cache[uri].getMessage(); //old one can be used
  }else{
    return res; // old one can't be used, get a new one back
  }
}


void handleGet(int server_fd, std::string r, int client_fd){
  Request req(r);
  std::string uri = req.getURI(); //get request target
  std::string res;
  if(Cache.find(uri)!=Cache.end()){ //resource is in cache
    Response s = Cache[uri];   //get response
    std::map<std::string,std::string> cacheField = s.getCacheField();
    if(cacheField.find("no-cache")!=cacheField.end()){ //if no-cache, revalidate
      revalidate(server_fd,s,uri);
      std::cout<<"in cache, but needs validation\n";
    }else{
      if(!checkFresh()){   //if not fresh
        std::cout<<"in cache, but expires\n";
        if(s.getLastModified()!=""||s.getEtag()!=""){
          res = revalidate(server_fd,s,uri);  //if has etag or last-modified, revalidate
        }else{
          res = ask_reply(r,server_fd);  // no etag or last-mpdified field, directly get from server
          saveRes(res,uri);  //save response
        }
      }else{
        std::cout<<"in cache, valid\n";
        res = s.getMessage();  //if fresh, use it
      }
    }  
  }else{
    std::cout<<"not in cache\n";
    res = ask_reply(r,server_fd); //if not in cache, directly get from server
    saveRes(res,uri); // save response
  }
 
  char req_msg[res.length()+1]={0};
  strcpy(req_msg,res.c_str());
  /*
  int start = 0;
  int new_len=0; 
  while(1){  // receive message in loop
    if((new_len=send(server_fd,&req_msg[start],res.length()+1-start,0))<=0){
      break;
    }
    start += new_len;   
    }*/
  int send_len;
  if((send_len=send(client_fd,req_msg,res.length()+1,0))>0){//send response back to browser
    std::cout<<"sending response back to browser\n";
  }
}
