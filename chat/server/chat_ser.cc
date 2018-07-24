#include "chat_ser.h"
#include "../common/util.hpp"
#include"api.hpp"
#include<sstream>
namespace server{
		int ChatServer::Start(const std::string& ip, short port)
		{
				//UDP¿¿¿¿¿¿
				sock_ = socket(AF_INET, SOCK_DGRAM, 0);
				if (sock_ < 0)
				{
						perror("socket");
						return -1;

				}
				sockaddr_in addr;
				addr.sin_family = AF_INET;
				addr.sin_addr.s_addr = inet_addr(ip.c_str());//string¿¿uint32
				addr.sin_port = htons(port);//¿¿¿¿¿¿¿¿
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
						server->RecvMsg();//¿¿¿¿¿¿¿

				}
				return NULL;

		}
		void* ChatServer::Consume(void* arg)
		{
				ChatServer* server = reinterpret_cast<ChatServer*> (arg);
				while (true)
				{
						server->BroadCast();//¿¿¿¿¿¿¿

				}
				return NULL;

		}
		int ChatServer::RecvMsg()
		{
				char buf[1024 * 5] = { 0  };
				sockaddr_in peer;
				socklen_t len = sizeof(peer);
				ssize_t read_size = recvfrom(sock_, buf, sizeof(buf)-1, 0, (sockaddr*)&peer, &len);
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
				queue_.PushBack(context);
				return 0;

		}
		int ChatServer::BroadCast()
		{
				Context context;
				std::string str;
				queue_.PopFront(&context);
				Data data;
				data.UnSerialize(context.str);
				if (data.cmd == "quit")
				{
						DelUser(context.addr, data.name);

				}
				else
				{
						AddUser(context.addr, data.name);

				}
				LOG(INFO) << "========friend_list=======\n";

				for (auto item : online_friend_list_)//range for ¿¿¿¿¿¿
				{
						LOG(INFO) << item.first << " " << inet_ntoa(item.second.sin_addr) << ":" << ntohs(item.second.sin_port);
						//¿¿¿¿¿¿¿¿¿
						SendMsg(context.str, item.second);

				}
				LOG(INFO) << "===============\n";
				return 0;

		}
		void ChatServer::AddUser(sockaddr_in addr, const std::string& name)
		{

				std::stringstream ss;//¿¿key,name+ip
				ss << name << ":" << addr.sin_addr.s_addr;
				online_friend_list_[ss.str()] = addr;//¿¿¿¿¿¿¿¿¿

		}
		void ChatServer::DelUser(sockaddr_in addr, const std::string&name)
		{
				std::stringstream ss;//¿¿key,name+ip
				ss << name << ":" << addr.sin_addr.s_addr;
				online_friend_list_.erase(ss.str());//¿¿¿¿¿¿¿¿¿¿

		}
		void ChatServer::SendMsg(const std::string&data, sockaddr_in addr)
		{
				sendto(sock_, data.data(), data.size(), 0, (sockaddr*)&addr, sizeof(addr));
				LOG(INFO) << "[Reponse] " << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << " " << data << "\n";

		}


}//end serve
