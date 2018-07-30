#include "chat_ser.h"
#include "../common/util.hpp"
#include"api.hpp"
#include<sstream>
namespace server{
  int ChatServer::Start(const std::string& ip, short port)
  {
	//UDP服务器的启动
    sock_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_ < 0)
    {
      perror("socket");
      return -1;

    }
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());//将string转成uint32类型
    addr.sin_port = htons(port);//将主机序转换为网络序
    int ret = bind(sock_, (sockaddr*)&addr, sizeof(addr));
    if (ret < 0)
    {
      perror("blind");
      return -1;

    }
    LOG(INFO) << "Server Start OK!\n";
  
    pthread_t productor, consumer;
    pthread_create(&productor, NULL, Product, this);
    pthread_create(&consumer, NULL, Consume, this);
    pthread_join(productor, NULL);
    pthread_join(consumer, NULL);
    return 0;

  }
  void* ChatServer::Product(void* arg)
  {
    ChatServer* server = reinterpret_cast<ChatServer*> (arg);
    while (true)
    {
		server->RecvMsg();//生产者一直接收消息

    }
    return NULL;

  }
  void* ChatServer::Consume(void* arg)
  {
    ChatServer* server = reinterpret_cast<ChatServer*> (arg);
    while (true)
    {
      server->BroadCast();//消费者一直广播消息

    }
    return NULL;

  }
  int ChatServer::RecvMsg()
  {
    char buf[1024 * 5] = { 0  };
    sockaddr_in peer;
    socklen_t len = sizeof(peer);
    ssize_t read_size = recvfrom(sock_, buf, sizeof(buf)-1, 0, (sockaddr*)&peer, &len);
	//将来自客户端的消息读到buf中
    if (read_size < 0)
    {
      perror("recvfrom");
      return -1;

    }
    LOG(INFO) << "productor read the msg: " << buf << "\n";
   
	buf[read_size] = '\0';
    Context context;
    context.str = buf;
    context.addr = peer;
    queue_.PushBack(context);//消息入到消息队列（已将序列化）
    return 0;

  }
  int ChatServer::BroadCast()
  {
    Context context;
    std::string str;
	queue_.PopFront(&context); 
    Data data;
    data.UnSerialize(context.str);//拿出消息进行反序列化
    if (data.cmd == "quit")
    {
      DelUser(context.addr, data.name);

    }
    else
    {
      AddUser(context.addr, data.name);

    }
    LOG(INFO) << "========friend_list=======\n";
   
	for (auto item : online_friend_list_)//遍历好友列表，给每个人都发消息
    {
      LOG(INFO) << item.first << " " << inet_ntoa(item.second.sin_addr) << ":" << ntohs(item.second.sin_port);
	  //日志打印出消息内容及发消息的人
      SendMsg(context.str, item.second);

    }
    LOG(INFO) << "===============\n";
    return 0;

  }
  void ChatServer::AddUser(sockaddr_in addr, const std::string& name)
  {

    std::stringstream ss;//构造key,name+ip
    ss << name << ":" << addr.sin_addr.s_addr;
    online_friend_list_[ss.str()] = addr;//如果有，替换，没有加入到好友列表(ip+port)

  }
  void ChatServer::DelUser(sockaddr_in addr, const std::string& name)
  {
    std::stringstream ss;//构造key,name+ip
    ss << name << ":" << addr.sin_addr.s_addr;
    online_friend_list_.erase(ss.str());//删除

  }
  void ChatServer::SendMsg(const std::string& data, sockaddr_in addr)
  {
    sendto(sock_, data.data(), data.size(), 0, (sockaddr*)&addr, sizeof(addr));
    LOG(INFO) << "[Reponse] " << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << " " << data << "\n";
  }                            //发给谁的ip                         port


}//end serve
