#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "proxy.h"
#include <sys/select.h>
#include <pthread.h>
#include "client_info.h"
#include <pthread.h>
#include <thread>
#include "expire.h"
#include <algorithm>
#include <unordered_map>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory>
//std::unordered_map<std::string,Response> Cache;
LRUCache Cache(20);
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
std::ofstream logFile("/var/log/erss/proxy.log");
void proxy::proxy_server_build(){
  int status;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = NULL;
  const char *port     = "12345";

  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family   = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags    = AI_PASSIVE;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    std::cerr << "Error: cannot get address info for host" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
  } //if

  socket_fd = socket(host_info_list->ai_family, 
		     host_info_list->ai_socktype, 
		     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    std::cerr << "Error: cannot create socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
  } //if
  
  int yes = 1;
  status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    std::cerr << "Error: cannot bind socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
  } //if

  status = listen(socket_fd, 100);
  if (status == -1) {
    std::cerr << "Error: cannot listen on socket" << std::endl; 
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
  } //if
  freeaddrinfo(host_info_list);
}


void proxy::connect_accept(){
  int id = 0;
  while(true){
    struct sockaddr_storage socket_addr;
    memset(&socket_addr, 0, sizeof socket_addr);
    socklen_t socket_addr_len = sizeof(socket_addr);
    int client_connection_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (client_connection_fd == -1) {
      std::cerr << "Error: cannot accept connection on socket" << std::endl;
      continue;
    } //if
    struct sockaddr_in *temp = (struct sockaddr_in *)&socket_addr;
    char* ip = inet_ntoa(temp->sin_addr);
    pthread_t thread;
    pthread_mutex_lock(&mutex);
    std::unique_ptr<Access> acc (new Access());
    Access* access = acc.release();
    //Access* access = new Access();
    access->setFd(client_connection_fd);
    access->setID(id);
    access->setIp(std::string(ip));
    id++;
    pthread_mutex_unlock(&mutex);
    pthread_create(&thread, NULL, handleRequest, (void*)access);

  }

}

int proxy::server_socket_connect(const char *hostname, const char *port){
  int status;
  int socket_fd1;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  
  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family   = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    std::cerr << "Error: cannot get address info for host" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    return -1;
  } //if

  socket_fd1 = socket(host_info_list->ai_family, 
		     host_info_list->ai_socktype, 
		     host_info_list->ai_protocol);
  if (socket_fd1 == -1) {
    std::cerr << "Error: cannot create socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    return -1;
  } //if
  
  std::cout << "Connecting to " << hostname << " on port " << port << "..." << std::endl;
  
  status = connect(socket_fd1, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    std::cout << "Error: cannot connect to socket" << std::endl;
    std::cout << "  (" << hostname << "," << port << ")" << std::endl;
    return -1;
  } //if
  freeaddrinfo(host_info_list);
  int temp = status;
  return socket_fd1;
}


void* proxy::handleRequest(void* access){
  char buffer[65536];
  Access* access_ = (Access*)access;
  int len = recv(access_->getFd(), buffer, 65536, 0);
  if(len<=0){
    pthread_mutex_lock(&mutex);
    logFile << access_->getID() << ": WARNING Invalid Request" << std::endl;
    pthread_mutex_unlock(&mutex);
    return NULL;
  }
  
  Request request = Request(std::string(buffer,len));//delete new
  pthread_mutex_lock(&mutex);
  logFile << access_->getID() << ": \"" << request.getStartLine() << "\" from "
          << access_->getIp() << " @ " << currentDateTime() << std::endl;
  pthread_mutex_unlock(&mutex);
  if(!request.checkFormat()){
    send(access_->getFd(),"HTTP/1.1 400 Bad Request\r\n\r\n",28,0);
    return NULL;
  }
  std::string hostname = request.getHost();
  std::string port = request.getPort();
  std::string method = request.getMethod();
  int new_connect_socket = server_socket_connect(hostname.c_str(), port.c_str());
  if(new_connect_socket==-1){
    std::cout<<"Cannot connect to web server\n";
    send(access_->getFd(),"HTTP/1.1 408 Request Timeout\r\n\r\n",30,0);
  }
  if(method=="CONNECT"){
    //connection(hostname.c_str(), port.c_str(), access);
    connection( new_connect_socket, access);
    
  }
  
  if(method=="GET"){
    handleGet(new_connect_socket,std::string(buffer,len),access_->getFd(), access_->getID());
    
    }
  if(method=="POST"){
    handlePost(new_connect_socket,std::string(buffer,len),access_->getFd(), access_->getID());
  }
  //
  pthread_mutex_lock(&mutex);
  logFile << access_->getID() << ": Tunnel closed" << std::endl;
  pthread_mutex_unlock(&mutex);
  return NULL;
}

void proxy::connection(int new_connect_socket, void *access){
  Access* access_ = (Access*)access;
  std::string OK_200("HTTP/1.1 200 OK\r\n\r\n");                                                                                                                  
  if (send(access_->getFd(), OK_200.c_str(), OK_200.length(), 0) == -1) {
      std::cout<<"send 200 OK back failed\n";
  }
  pthread_mutex_lock(&mutex);
  logFile << access_->getID() << ": Responding \"HTTP/1.1 200 OK\"" << std::endl;
  pthread_mutex_unlock(&mutex);
  fd_set readfds;
  while(true){
    int  n;
    //char buf1[256], buf2[256];
    // clear the set ahead of time
    FD_ZERO(&readfds);
    // add our descriptors to the set
    int access_fd = access_->getFd();
    FD_SET(access_fd, &readfds);
    FD_SET(new_connect_socket, &readfds);
    n = new_connect_socket + 1;

    // wait until either socket has data ready to be recv()d (timeout 10.5 secs)
    //tv.tv_sec = 10;
    //tv.tv_usec = 500000;
    int rv = select(n, &readfds, NULL, NULL, NULL);
    int len;
    if (rv == -1) {
        perror("select"); // error occurred in select()
    }
    else if (rv == 0) {   
        break;
    }
    char message[65536] = {0};
    if (FD_ISSET(access_fd, &readfds)) {
        len = recv(access_fd, message, sizeof(message), 0);
        if (len < 0) {
            std::cout<<"Failed to recv from browser\n";
            break;
        } else if (len == 0) {
            break;
        }

        len = send(new_connect_socket, message, len, 0);
        if (len < 0) {
            std::cout<<"Failed to send to server\n";
            break;
        }
    } 
    else if (FD_ISSET(new_connect_socket, &readfds)) {
      len = recv(new_connect_socket, message, sizeof(message), 0);
        if (len < 0) {
            std::cout<<"Failed to recv from web\n";
            break;
        } else if (len == 0) {
            break;
        }
        
        len = send(access_fd, message, len, 0);
        
        if (len < 0) {
            std::cout<<"Failed to send to client\n";
            break;
        }
    }
    
  }
  
}
size_t recvChunk(int server_fd,std::vector<std::vector<char>>& buff){
  const char * end = "0\r\n\r\n";
  int recvSize=0;
  while(true){
    int singleRecvSize = 0;
    char single_buff[65536] = {0};
    if((singleRecvSize=recv(server_fd,single_buff,65536,0))<=0){
      break;
    }
    std::vector<char> temp(single_buff,single_buff+singleRecvSize);
    buff.push_back(temp);
    recvSize+=singleRecvSize;
    if(search(temp.begin(),temp.end(),end,end+strlen(end))-temp.end()!=0){
      break;
    }
  }
  return recvSize;
}
int receive(int server_fd,std::vector<std::vector<char>>& buff){
  int recvSize=0;
  char single_buff[30000] = {0};
  if((recvSize=recv(server_fd,single_buff,65536,0))<=0){
      return recvSize;
  }
  std::vector<char> temp(single_buff,single_buff+recvSize);
  buff.push_back(temp);
  Response res(buff);
  int contentLen = res.getContentLength();
  int headerLen = res.getHeaderSize();
  if(res.getField("Transfer-Encoding: ")=="chunked"){
    return recvChunk(server_fd,buff);
  }
  while(recvSize<(headerLen+contentLen-1)){
    int singleRecvSize = 0;
    char single_buff[65536] = {0};
    if((singleRecvSize=recv(server_fd,single_buff,65536,0))<=0){
      break;
    }
    std::vector<char> temp(single_buff,single_buff+singleRecvSize);
    buff.push_back(temp);
    recvSize+=singleRecvSize;
  }
  return recvSize;
}

int sendMessage(int client_fd,std::vector<std::vector<char>> buff){
  int sendSize = 0;
  for(size_t i =0;i<buff.size();i++){
    int singleSendSize;
    if(buff[i].size()>0){
      if((singleSendSize = send(client_fd,&buff[i].data()[0],buff[i].size(),0))<=0){
        return -1;
      }
      sendSize+=singleSendSize;
    }
  }
  return sendSize;
}

std::vector<std::vector<char>> ask_reply(std::string req,int server_fd){
  int send_len;
  char req_msg[req.length()+1]={0};
  strcpy(req_msg,req.c_str());
  send_len=send(server_fd,req_msg,req.length()+1,0);
  std::vector<std::vector<char>> buff;
  int recvSize = receive(server_fd,buff);
  return buff;
  
}

bool checkCacheble(Response s, int ID){
  std::map<std::string,std::string> cacheField = s.getCacheField();
  if(cacheField.find("no-store")!=cacheField.end()){
    pthread_mutex_lock(&mutex);
    logFile << ID <<": not cacheable because no-store"<<std::endl;
    pthread_mutex_unlock(&mutex);
    return false;
  }
  if(cacheField.find("private")!=cacheField.end()){
    pthread_mutex_lock(&mutex);
    logFile << ID <<": not cacheable because private"<<std::endl;
    pthread_mutex_unlock(&mutex);
    return false;
  }
  return true;
}

void saveRes(std::vector<std::vector<char>> buff, std::string uri, int ID){
  if(buff.size()==0){
    return;
  }
  std::vector<char> temp =buff[0];
  std::string headerPart(temp.begin(),temp.end());
  if(headerPart.find("HTTP/1.1 200 OK\r\n")!=std::string::npos){//cache response that is 200 ok
    Response s(buff);
    if(checkCacheble(s, ID)){
      pthread_mutex_lock(&mutex);
      Cache.put(uri, s);
      pthread_mutex_unlock(&mutex);
    }
  } 
}

std::vector<std::vector<char>> revalidate(int server_fd, Response s, Request r, int id){
  pthread_mutex_lock(&mutex);
  logFile << id << ": in cache, requires validation" << std::endl;
  pthread_mutex_unlock(&mutex);
  std::string req;
  std::string uri = r.getURI();
  if(s.getLastModified()!=""){
    req = s.makeModifiedRequest(r);      
  }
  if(s.getEtag()!=""){
    req = s.makeEtagRequest(r);
  }
  std::vector<std::vector<char>> res = ask_reply(req,server_fd);//ask server if the response can still be used

  if(res.size()==0){
    return res;
  }
  saveRes(res, uri, id); //get the response from server
  std::string headerPart = res[0].data();
  if(headerPart.find("HTTP/1.1 304")!=std::string::npos){
    pthread_mutex_lock(&mutex);
    logFile << id << ": Requesting \""<< r.getStartLine()<<"\" from " << r.getHost() << std::endl;
    pthread_mutex_unlock(&mutex);
    pthread_mutex_lock(&mutex);
    logFile << id << ": Received \"" << s.getStartLine() <<"\" from " << r.getHost() << std::endl;
    pthread_mutex_unlock(&mutex);
    pthread_mutex_lock(&mutex);
    Response temp = Cache.get(uri);
    pthread_mutex_unlock(&mutex);
    return temp.getMessage(); //old one can be used
  }else{
    return res; // old one can't be used, return the new one back
  }
}
bool checkFresh(Response res, int id){
  std::string expire = res.getExpires();
  std::string date = res.getDate();
  
  std::map<std::string,std::string> cacheField = res.getCacheField();
  if(cacheField.find("max-age")!=cacheField.end()){
    tm_Time data_time = tm_Time(date);
    int max_age = std::stoi(cacheField.find("max-age")->second);
    if(max_age>0){
      time_t curr_time = time(0);
      tm *curr = gmtime(&curr_time);
      time_t date_time_ = mktime(&data_time.time);
      if(date_time_+ max_age >= curr_time+18000){
        return true; 
      }
    }
    time_t expire_t = mktime(&data_time.time)+max_age;
    pthread_mutex_lock(&mutex);
    logFile << id << ": in cache, but expired at " << expectedTime(expire_t) << std::endl;
    pthread_mutex_unlock(&mutex);
    return false;
  }
  if(expire!=""){
    tm_Time expire_time = tm_Time(expire);
    time_t curr_time = time(0);
    expire_time.print();
    if(mktime(&expire_time.time) > curr_time+18000){
      return true;
    }
    pthread_mutex_lock(&mutex);
    logFile << id << ": in cache, but expired at " << expectedTime(mktime(&expire_time.time)) << std::endl;
    pthread_mutex_unlock(&mutex);
    return false;
  }
  if(res.getLastModified()!=""){
    tm_Time data_time = tm_Time(date);
    tm_Time last_modified_time = tm_Time(res.getLastModified());
    time_t curr_time = time(0);
    time_t maxage = mktime(&data_time.time)-mktime(&last_modified_time.time)*0.1;
    if(curr_time+18000 <= maxage + mktime(&data_time.time)){
      return true;
    }
    time_t expire_t = mktime(&data_time.time)+maxage;
    pthread_mutex_lock(&mutex);
    logFile << id << ": in cache, but expired at " << expectedTime(expire_t) << std::endl;
    pthread_mutex_unlock(&mutex);
    return false;
  }
  return true;
}

void proxy::handleGet(int server_fd, std::string r, int client_fd, int ID){
  Request req(r);
  std::string uri = req.getURI(); //get request target
  std::vector<std::vector<char>> res;
  pthread_mutex_lock(&mutex);
  Response temp = Cache.get(uri);
  pthread_mutex_unlock(&mutex);
  if(temp.getMessage().size()!=0){ //resource is in cache
    Response s = temp;   //get response
    std::map<std::string,std::string> cacheField = s.getCacheField();
    if(cacheField.find("no-cache")!=cacheField.end()){ //if no-cache, revalidate
      res=revalidate(server_fd, s, req, ID);
      pthread_mutex_lock(&mutex);
      logFile << ID<<": in cache, but requires re-validation"<<std::endl;
      pthread_mutex_unlock(&mutex);
    }else{
      if(!checkFresh(s, ID)){   //if not fresh
        if(s.getLastModified()!=""||s.getEtag()!=""){
          res = revalidate(server_fd, s, req, ID);  //if has etag or last-modified, revalidate
        }else{
          pthread_mutex_lock(&mutex);
          logFile << ID << ": in cache, valid " << std::endl;
          pthread_mutex_unlock(&mutex);
          res = ask_reply(r, server_fd);  // no etag or last-mpdified field, directly get from server
          saveRes(res, uri, ID);  //save response
        }
      }else{
        pthread_mutex_lock(&mutex);
        logFile << ID << ": in cache, valid " << std::endl;
        pthread_mutex_unlock(&mutex);
        res = s.getMessage();  //if fresh, use it
      }
    }  
  }else{
    pthread_mutex_lock(&mutex);
    logFile << ID << ": not in cache" << std::endl;
    pthread_mutex_unlock(&mutex);
    res = ask_reply(r,server_fd); //if not in cache, directly get from server
    saveRes(res,uri, ID); // save response
  }
  Response resp(res);
  
  if(!resp.checkFormat()){
    send(client_fd,"HTTP/1.1 502 Bad Gateway\r\n\r\n",28,0);
    return;
  }
  
  int send_len;
  if((send_len=sendMessage(client_fd,res))>0){
    pthread_mutex_lock(&mutex);
    logFile << ID << ": Responding \""<< resp.getStartLine() << "\"" << std::endl;
    pthread_mutex_unlock(&mutex);
  }
  
}
void proxy::handlePost(int server_fd,std::string r, int client_fd, int ID){
  Request req(r);
  std::string uri = req.getURI(); //get request target
  std::vector<std::vector<char>> res;
  res = ask_reply(r,server_fd);
  Response resp(res);
  /*
  if(!resp.checkFormat()){
    std::cout<<"Bad gateway\n";
    send(client_fd,"HTTP/1.1 502 Bad Gateway\r\n\r\n",28,0);
    return;
  }
  */
  int send_len;
  if((send_len=sendMessage(client_fd,res))>0){
    pthread_mutex_lock(&mutex);
    logFile << ID << ": Responding \""<< resp.getStartLine() << "\"" << std::endl;
    pthread_mutex_unlock(&mutex);
  }
}

int main(void){
    proxy *myproxy = new proxy();
    myproxy->proxy_server_build();
    myproxy->connect_accept();
    //myproxy->proxy_close();
}
