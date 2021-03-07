#include <string>
#include <vector>
class Request{
  std::string message;
  std::string startLine;
  std::vector<std::string> splitLine;
public:
  Request(std::string _message);
  std::vector<std::string> split(std::string line, char delimiter);
  std::string setLine(std::string info);
  std::string getMethod();
  bool checkFormat();
  std::string getURI();
  std::string getHost();
  std::string getPort();
  std::string getStartLine();
  std::string getHeader();
};


