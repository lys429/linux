#pragma once 
#include<string>
#include<unordered_map>
#include "block_queue.hpp"
#include<sys/socket.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
namespace server
{
	struct Context{
    std::string str;
    sockaddr_in addr;

  };
  class ChatServer
  {
    public:
      int Start(const std::string& ip, short port);
      int RecvMsg();
      int BroadCast();
	  static void* Consume(void*);
	  static void* Product(void*);
	  void AddUser(sockaddr_in addr, const std::string& name);
	  void DelUser(sockaddr_in addr, const std::string& name);
	  void SendMsg(const std::string&str, sockaddr_in addr);
    protected:
	  std::unordered_map<std::string, sockaddr_in> online_friend_list_;//hash_map
      BlockQueue<Context> queue_;
      int sock_;
  };
}//end server
