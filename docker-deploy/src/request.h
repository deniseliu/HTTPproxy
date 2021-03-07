class request {
    public:
        int socket_fd;
        std::string from_ip;
        int id;
        
        request(int socked_fd, std::string from_ip, int id){
	  this->socket_fd = socked_fd;
	  this->from_ip = from_ip;
	  id = 1;
	}
};
