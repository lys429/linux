#include"chat_cl.h"
#include<iostream>
namespace client{
  int ChatClient::Init(const std::string& server_ip, short server_port)
  {
    sock_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_ < 0)
    {
      perror("socket");
      return -1;

    }
    server_addr_.sin_family = AF_INET;
    server_addr_.sin_addr.s_addr = inet_addr(server_ip.c_str());
    server_addr_.sin_port = htons(server_port);
    return 0;

  }
  int ChatClient::SetUserInfo(const std::string& name, const std::string& school)
  {
    name_ = name;
    school_ = school;
    return 0;

  }
  void ChatClient::SendMsg(const std::string& msg)
  {
	//按照服务器的格式进行转换，写到data中
    server::Data data;
    data.name = name_;
    data.school = school_;
    data.msg = msg;
    if (data.msg == "quit" || data.msg == "exit" || data.msg == "Q")
    {
      data.cmd = "quit";
    }
    std::string str;
    data.Serialize(&str);//序列化消息
    sendto(sock_, str.c_str(), str.size(), 0, (sockaddr*)& server_addr_, sizeof(server_addr_));
	//往sock_发送消息
    return;

  }
  /*void Print(const std::string&log)
  {
    FILE*fp = fopen("./log.txt", "a");
    fwrite(log.c_str(), log.size(), 1, fp);
    fclose(fp);

  }*/
  void ChatClient::RecvMsg(server::Data* data)
  {

    char buf[1024 * 10] = { 0 };
    ssize_t read_size = recvfrom(sock_, buf, sizeof(buf)-1, 0, NULL, NULL);
    if (read_size < 0)
    {
      perror("recvfrom");
      return;

    }
    buf[read_size] = '\0';
    //Print(buf);
    data->UnSerialize(buf);//反序列化
    return;


  }

}//end client 

//#include<iostream>
//void* Send(void* arg)
//{
//  client::ChatClient* client = reinterpret_cast<client::ChatClient*>(arg);
//  while (true)
//  {
//    std::string str;
//    std::cin >> str;
//    client->SendMsg(str);
//
//  }
//  return NULL;
//
//}
//void* Recv(void* arg)
//{
//  client::ChatClient* client = reinterpret_cast<client::ChatClient*>(arg);
//  while (true)
//  {
//    server::Data data;
//    client->RecvMsg(&data);
//    std::cout << "{" << data.name.c_str() << "|" << data.school.c_str() << "}" << data.msg.c_str() << "\n";
//
//  }
//  return NULL;
//
//}
//
//int main(int argc, char*argv[])
//{
//  if (argc != 3)
//  {
//    printf("Usage ./client [ip] [port]\n");
//    return 1;
//
//  }
//  client::ChatClient client;
//  client.Init(argv[1], atoi(argv[2]));
//  std::string name, school;
//  std::cout << "输入用户名：";
//  std::cin >> name;
//  std::cout << "输入学校：";
//  std::cin >> school;
//  client.SetUserInfo(name, school);
//
//  pthread_t stid, rtid;
//  pthread_create(&stid, NULL, Send, &client);
//  pthread_create(&rtid, NULL, Recv, &client);
//  pthread_join(stid, NULL);
//  pthread_join(rtid, NULL);
//  return 0;
//
//}
