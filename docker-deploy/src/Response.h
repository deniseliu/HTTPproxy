#include <string>
#include <vector>
#include <map>
#include <iostream>
#include "Request.h"
class Response{
  std::string startLine;
  std::vector<std::string> splitLine;
  std::map<std::string,std::string> cacheField;
  std::string cacheControlHeader;
  std::string etag;
  std::string expires;
  std::string last_modified;
  std::vector<std::vector<char>> message;
  std::string headerPart;
  void initCacheField();
  void setupCacheField();
  std::string getValue(std::string s);
public:
  Response(){};
  Response(std::vector<std::vector<char>> message);
  std::string getField(std::string header);
  std::vector<std::string> split(std::string line, char delimiter);
  std::vector<std::vector<char>> getMessage();
  std::string getStartLine();
  bool checkFormat();
  std::map<std::string,std::string> getCacheField();
  std::string getEtag();
  std::string getExpires();
  std::string getLastModified();
  std::string getDate();
  int getHeaderSize();
  std::string makeEtagRequest(Request r);
  std::string makeModifiedRequest(Request r);
  bool checkChunkSize(std::string body);
  bool checkContentLength(std::string body);
  bool notCorrupted();
  size_t getContentLength();

};
