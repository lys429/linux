
//#include "../server/api.hpp"
#include<string>
#include<sys/socket.h>
#include<stdio.h>
#include<stdlib.h>
#include "chat_cl.h"

#include"window.h"
#include<pthread.h>
#include<signal.h>
#include<stdlib.h>
#include"../server/api.hpp"
#include"../common/util.hpp"
client::ChatClient* g_client = NULL;//定义一个全局变量；也可以使用单例莫属进行封装
client::Window* g_window = NULL;
pthread_mutex_t g_lock;

void* Send(void*arg)
{
  (void)arg;
  while (1)
  {
    std::string msg;
    pthread_mutex_lock(&g_lock);
    g_window->DrawInput();
    pthread_mutex_unlock(&g_lock);
    g_window->GetStrFromWin(g_window->input_win_, &msg);
    g_client->SendMsg(msg);

  }
  return NULL;

}

void* Recv(void*arg)
{
  (void)arg;
  while (1)
  {
    pthread_mutex_lock(&g_lock);
    g_window->DrawOutput();
    pthread_mutex_unlock(&g_lock);
    g_window->DrawFriendList();
    server::Data data;
    g_client->RecvMsg(&data);
    if (data.cmd == "quit")
    {
      g_window->DelFriend(data.name + "|" + data.school);

    }
    else
    {
      g_window->AddFriend(data.name + "|" + data.school);
      g_window->AddMsg(data.name + ":" + data.msg);

    }

  }

}
void Quit(int sig)
{
  (void)sig;
  g_client->SendMsg("quit");
  delete g_window;
  delete g_client;
  exit(0);

}
void Run(const std::string&ip, short port)
{
  signal(SIGINT, Quit);
  pthread_mutex_init(&g_lock, NULL);
  g_client = new client::ChatClient();
  int ret = g_client->Init(ip, port);
  if (ret < 0)
  {
    LOG(ERROR) << "client Init failed!\n";
    return;

  }
  std::string name, school;
  std::cout << "请�入用户名：" << std::endl;
  std::cin >> name;
  std::cout << "请输入学校：" << std::endl;
  std::cin >> school;
  g_client->SetUserInfo(name, school);
  g_window = new client::Window();
  g_window->DrawHeader();

  pthread_t stid, rtid;
  pthread_create(&stid, NULL, Send, NULL);
  pthread_create(&rtid, NULL, Recv, NULL);
  pthread_join(stid, NULL);
  pthread_join(rtid, NULL);
  delete g_client;
  delete g_window;
  pthread_mutex_destroy(&g_lock);

}

int main(int argc, char*argv[])
{
  if (argc != 3)
  {
    LOG(ERROR) << "Usage ./chat_client [ip] [port]\n";
    return 1;

  }
  Run(argv[1], atoi(argv[2]));
  return 0;

} 
