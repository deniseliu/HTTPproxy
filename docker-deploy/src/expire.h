#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
class Monthmap {
 public:
  std::map<std::string, int> Month_map;

 public:
  Monthmap() {
    Month_map.insert(std::pair<std::string, int>("Jan", 1));
    Month_map.insert(std::pair<std::string, int>("Feb", 2));
    Month_map.insert(std::pair<std::string, int>("Mar", 3));
    Month_map.insert(std::pair<std::string, int>("Apr", 4));
    Month_map.insert(std::pair<std::string, int>("May", 5));
    Month_map.insert(std::pair<std::string, int>("Jun", 6));
    Month_map.insert(std::pair<std::string, int>("Jul", 7));
    Month_map.insert(std::pair<std::string, int>("Aug", 8));
    Month_map.insert(std::pair<std::string, int>("Sep", 9));
    Month_map.insert(std::pair<std::string, int>("Oct", 10));
    Month_map.insert(std::pair<std::string, int>("Nov", 11));
    Month_map.insert(std::pair<std::string, int>("Dec", 12));
  }
  int getMap(std::string str) { return Month_map.find(str)->second; }
};

class Daymap {
 public:
  std::map<std::string, int> Day_map;

 public:
  Daymap() {
    Day_map.insert(std::pair<std::string, int>("Sun", 0));
    Day_map.insert(std::pair<std::string, int>("Mon", 1));
    Day_map.insert(std::pair<std::string, int>("Tue", 2));
    Day_map.insert(std::pair<std::string, int>("Wed", 3));
    Day_map.insert(std::pair<std::string, int>("Thu", 4));
    Day_map.insert(std::pair<std::string, int>("Fri", 5));
    Day_map.insert(std::pair<std::string, int>("Sat", 6));
  }
  int getMap(std::string str) { return Day_map.find(str)->second; }
};



class tm_Time{
  public:
    struct tm time;
  public:
    tm_Time(){}
    tm_Time(std::string expire){
        Monthmap month;
        Daymap day;
        time.tm_sec = atoi(expire.substr(23, 2).c_str()); 
        time.tm_min = atoi(expire.substr(20, 2).c_str());
        time.tm_hour = atoi(expire.substr(17, 2).c_str());
        time.tm_mday = atoi(expire.substr(5, 2).c_str());
        time.tm_mon = month.getMap(expire.substr(8, 3).c_str()) - 1;
        time.tm_year = atoi(expire.substr(12, 4).c_str()) - 1900;
        time.tm_isdst = 0;
    }
    void print(){
      std::cout<<"year:"<<time.tm_year+1900<<"\n";
      std::cout<<"month:"<<time.tm_mon<<"\n";
      std::cout<<"day:"<<time.tm_mday<<"\n";
      std::cout<<"hour:"<<time.tm_hour<<"\n";
      std::cout<<"min:"<<time.tm_min<<"\n";
      std::cout<<"sec:"<<time.tm_sec<<"\n";
      
    }

};



