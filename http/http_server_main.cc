#include<iostream>
#include"http_server.h"
using namespace http_server;
int main(int argc, char*argv[])
{
	if (argc != 3)
	{
		std::cout << "Usage ./server [ip] [port]" << std::endl;
		return 1;
	}
	HttpServer server;
	server.Start(argv[1], atoi(argv[2]));
	return 0;
}