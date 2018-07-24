#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string>
#include"../server/api.hpp"
typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;

namespace client{
  class ChatClient
  {
    public:

      int Init(const std::string& server_ip, short server_port);//初始化客户端
      int SetUserInfo(const std::string& name, const std::string& school);//置用户信息
      void SendMsg(const std::string&msg);
      void RecvMsg(server::Data*data);
    private:
      int sock_;
      sockaddr_in server_addr_;
      std::string name_;
      std::string school_;

  };

}
