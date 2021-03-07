#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/select.h>
#include "Access.h"
#include "Response.h"
#include <unordered_map>

class proxy{
    private:
        int socket_fd;
	    int request_num;
        //int client_connection_fd;
        const char *port;
    public: 
        proxy(){
            socket_fd=0;
	        request_num = 1;
            port = NULL;
        }
        void proxy_close(){
            close(this->socket_fd);
        }
        void proxy_server_build();
        static int server_socket_connect(const char *hostname, const char *port);
  //static void connection(const char *hostname, const char *port, void *access);
        static void connection(int new_connect_socket, void *access);
         void connect_accept();
        //void handleRequest(Access access);
        static void* handleRequest(void* access);
  static void handleGet(int server_fd, std::string r, int client_fd, int ID);
  static void handlePost(int server_fd, std::string r, int client_fd, int ID);
};



struct DLinkedNode {
    std::string key;
    Response value;
    DLinkedNode* prev;
    DLinkedNode* next;
    DLinkedNode(): prev(nullptr), next(nullptr) {
        key = "";
        std::vector<std::vector<char>> buff;
        value = Response(buff);
    }
    DLinkedNode(std::string _key, Response _value): key(_key), value(_value), prev(nullptr), next(nullptr) {}
};

class LRUCache {
private:
    std::unordered_map<std::string, DLinkedNode*> cache;
    DLinkedNode* head;
    DLinkedNode* tail;
    int size;
    int capacity;

public:
    LRUCache(int _capacity): capacity(_capacity), size(0) {
        head = new DLinkedNode();
        tail = new DLinkedNode();
        head->next = tail;
        tail->prev = head;
    }
    
    Response get(std::string key) {
        if (!cache.count(key)) {
            std::vector<std::vector<char>> buff;
            return Response(buff);
        }
        DLinkedNode* node = cache[key];
        moveToHead(node);
        return node->value;
    }
    
    void put(std::string key, Response value) {
        if (!cache.count(key)) {
            DLinkedNode* node = new DLinkedNode(key, value);
            cache[key] = node;
            addToHead(node);
            ++size;
            if (size > capacity) {
                DLinkedNode* removed = removeTail();
                cache.erase(removed->key);
                delete removed;
                --size;
            }
        }
        else {
            DLinkedNode* node = cache[key];
            node->value = value;
            moveToHead(node);
        }
    }

    void addToHead(DLinkedNode* node) {
        node->prev = head;
        node->next = head->next;
        head->next->prev = node;
        head->next = node;
    }
    
    void removeNode(DLinkedNode* node) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    void moveToHead(DLinkedNode* node) {
        removeNode(node);
        addToHead(node);
    }

    DLinkedNode* removeTail() {
        DLinkedNode* node = tail->prev;
        removeNode(node);
        return node;
    }
};



// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}

const std::string expectedTime(time_t time){
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&time);
    
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}
