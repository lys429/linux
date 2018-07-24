#include"http_server.h"
#include<iostream>
using namespace std;

int main()
{
	http_server::HttpServer server;
	int ret = server.Start("0", 9090);
	std::cout << "ret:" << ret << std::endl;
	return 0;
}