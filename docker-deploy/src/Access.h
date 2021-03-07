#include <pthread.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

class Access {
 private:
  int id;
  int client_fd;
  std::string ip;

 public:
 Access(){
     this->id = -1;
     this->client_fd = -1;
 }
  void setFd(int my_client_fd) { this->client_fd = my_client_fd; }
  int getFd() { return this->client_fd; }
  void setID(int myid) { this->id = myid; }
  int getID() { return this->id; }
  void setIp(std::string ip){this->ip = ip;}
  std::string getIp(){return this->ip;}
};