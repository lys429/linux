#include<iostream>
#include<string>
#include<sstream>
#include<stdlib.h>
#include "util.hpp"

//这个代码要生成的就是CGI程序，通过这个CGI程序来完成具体的服务
//这个CGI程序仅仅完成简单的两个数的加法计算
void HttpResponse(const std::string&body)
{
	std::cout << "Content-Length: " << body.size() << "\n";
	std::cout << "\n";//这个是空行
	std::cout << body;
	return;
}

int main()
{
	const char* method = getenv("REQUEST_METHOD");
	if (method == NULL)
	{
		HttpResponse("NO env REQUEST_METHOD!");
		return 1;
	}
	StringUtil::UrlParam params;
	//2.如果是GET请求，那么就要从QUERY_STRING 中读取到参数
	if (std::string(method) == "GET")
	{
		const char*query_string = getenv("QUERY_STRING");
		StringUtil::ParseUrlParam(query_string, &params);
	}
	else if (std::string(method) == "POST")
  {
		//3.如果是POST请求，从CONTENT-LENGTH中读取body的长度，根据body的长度，从标准输入中读取请求的body
		// const char* content_length = getenv("CONTENT_LENGTH");
		//4.解析query_string或者body中的数据
		char buf[1024 * 10] = { 0 };
		read(0, buf, sizeof(buf)-1);
		StringUtil::ParseUrlParam(buf, &params);
	}
	//5.根据业务需要进行计算，此处的计算是计算a+b的值
	int a = std::stoi(params["a"]);
	int b = std::stoi(params["b"]);
	int result = a + b;
	std::cout << result << std::endl;
	//6.根据计算结果，构造响应的数据，协会到标准输出中
	std::stringstream ss;
	ss << "<h1> result = " << result << "</h1>";
  HttpResponse(ss.str());
	return 0;

}
