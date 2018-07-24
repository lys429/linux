#include"http_server.h"
#include"util.hpp"
#include<pthread.h>
#include<sys/wait.h>
namespace http_server{
	void* HttpServer::ThreadEntry(void* arg)//不能直接把这个入口函数定义为类的成员函数，因为掉用类成员函数要用对象
	{
		//准备工作
		Context*context = reinterpret_cast<Context*>(arg);
		HttpServer* server = context->server;
		//1.从文联描述符中读取数据，转换成Request对象
		int ret = 0;
		ret = context->server->ReadOneRequest(context);
		if (ret < 0)
		{
			LOG(ERROR) << "ReadOneRequest error!" << "\n";
			//用这个函数构造一个404 的http响应对象
			server->Process404(context);
			goto END;
		}
		//TODO test 通过以下函数将一个解析除开的请求打印出来
		context->server->PrintRequest(context->req);
		//2.把Request对象计算生成Response对象
		ret = server->HandlerRequest(context);
		if (ret < 0)
		{
			LOG(ERROR) << "ReadOneRequest error!" << "\n";
			//用这个函数构造一个404 的http响应对象
			server->Process404(context);
			goto END;
		}
	END:
		//TODO  处理失败的情况
		// 3.把Response对象进行序列化，写到客户端
		server->WriteOneResponse(context);
		//收尾工作
		close(context->new_sock);
		delete context;
		return NULL;
	}
	int HttpServer::Start(const std::string&ip, short port)
	{
		int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (listen_sock < 0)
		{
			perror("socket");
			return -1;
		}
		//要给socket加上一个选项，能够重新用我们的连接，就是那个bind状态
		//setsocketopt() 这个函数可以给socket加上一个选项
		int opt = 1;
		setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(ip.c_str());
		addr.sin_port = htons(port);
		int ret = bind(listen_sock, (sockaddr*)&addr, sizeof(addr));
		if (ret < 0)
		{
			perror("bind");
			return -1;
		}
		ret = listen(listen_sock, 5);
		if (ret < 0)
		{
			perror("listen");
			return -1;
		}
		//printf("ServerStart OK!\n");
		LOG(INFO) << "ServerStart OK!\n";
		while (1)
		{
			//基于多线程来实现一个TCP服务器
			sockaddr_in peer;
			socklen_t len = sizeof(peer);
			int new_sock = accept(listen_sock, (sockaddr*)&peer, &len);
			if (new_sock < 0)
			{
				perror("accept");
				continue;//这里不直接return，服务器在accept失败之后应该可以在次进行请求
			}
			//创建新线程，使用新线程完成这次请求的计算
			Context*context = new Context();
			context->new_sock = new_sock;
			context->server = this;
			pthread_t tid;
			pthread_create(&tid, NULL, ThreadEntry, reinterpret_cast<Context*>(context));
			pthread_detach(tid);
		}
		close(listen_sock);
		return 0;
	}
	//根据http请求字符串，来进行反序列化，从socket中读取一个字符串，输出Request对象
	int HttpServer::ReadOneRequest(Context*context)
	{
		Request* req = &context->req;
		//1.从socket中读取一行数据作为Request的首行，按行读取的分隔符应该就是\n \r \r\n
		std::string first_line;
		FileUtil::ReadLine(context->new_sock, &first_line);
		//2.解析首行，获取到请求的method和url
		int ret = ParseFirstLine(first_line, &req->method, &req->url);
		if (ret < 0)
		{
			LOG(ERROR) << "ParseFirstLine error! first_line=" << first_line << "\n";
			return -1;
		}
		//3.解析url，获取到url_pth，和query_string
		ret = ParseUrl(req->url, &req->url_path, &req->query_string);
		if (ret < 0)
		{
			LOG(ERROR) << "ParseUrl error! url=" << req->url << "\n";
			return -1;
		}
		//4.循环按行读取数据，每次读取到一行数据，就进行一次header对的解析，读到空行，说明header解析完毕
		std::string header_line;
		while (1)
		{
			FileUtil::ReadLine(context->new_sock, &header_line);
			//TODO 如果header_line是空行，就退出循环
			if (header_line == "")
			{
				break;
			}
			//因为ReadLine返回的header_line不包含\n等分割符，因此读到空行的时候，header_line就是空字符串
			ret = ParseHeader(header_line, &req->header);
			if (ret < 0)
			{
				LOG(ERROR) << "ParseHeader error! header_line = " << header_line << "\n";
				return -1;
			}
		}
		//5.如果是POST请求，但是没有Content-Length字段，认为这个请求失败
		Header::iterator it = req->header.find("Content-Length");
		if (req->method == "POST" && it == req->header.end())
		{
			LOG(ERROR) << "POST Request has no Vontent-Length!\n";
			return -1;
		}
		//如果是GET请求，就不用读body
		if (req->method == "GET")
		{
			return 0;
		}
		//如果是POST请求，并且header中包含了Content-Length字段，继续获取socket，获取到body的内容
		int content_length = atoi(it->second.c_str());
		ret = FileUtil::ReadN(context->new_sock, content_length, &req->body);
		if (ret < 0)
		{
			LOG(ERROR) << "ReadN error! content_length=" << content_length << "\n";
			return -1;
		}
		return 0;
	}
	//实现序列化，把Response对象转换成一个string，写回到socket中
	//这个函数的细节可能有很大的差异，但是只要能遵守http协议就好
	int HttpServer::WriteOneResponse(Context*context)
	{
		Response& resp = context->resp;
		std::stringstream ss;
		ss << "HTTP/1.1" << resp.code << " " << resp.desc << "\n";
		if (resp.cgi_resp == ""){
			//当前认为是在处理静态页面
			for (auto item : resp.header)
			{
				ss << item.first << ":" << item.second << "\n";
			}
			ss << "\n";
			ss << resp.body;
		}
		else
		{
			//当前是在处理CGI生成的页面,cgi_resp 同时包含了相应数据的header空行和body
			ss << resp.cgi_resp;
		}
		//2.将序列化的结果写入到socket中
		const std::string&str = ss.str();//获取到stringstream里面包含的string对象（使用了引用，并没有拷贝）
		write(context->new_sock, str.c_str(), str.size());
		return 0;
	}
	//根据Request对象,计算生成Response对象
	//1.静态文件
	//a)是GET 并且没有query_string作为参数
	//2.动态生成页面
	//a)GET 并且存在 query_string 作为参数
	// b)POST请求

	int HttpServer::HandlerRequest(Context*context)
	{
		const Request&req = context->req;
		Response*resp = &context->resp;
		resp->code = 200;
		resp->desc = "OK";
		//判断当前的处理方式是按照静态文件处理还是动态生成
		if (req.method == "GET" && req.query_string == "")
		{
			return context->server->ProcessStaticFile(context);
		}
		else if ((req.method == "GET" && req.query_string != "") || req.method == "POST")
		{
			return context->server->ProcessCGI(context);
		}
		else
		{
			LOG(ERROR) << "Unsupport Method! method=" << req.method << "\n";
			return -1;
		}
		return -1;
	}
	int HttpServer::Process404(Context*context)
	{
		Response*resp = &context->resp;
		resp->code = 404;
		resp->desc = "Not Found";
		resp->body = "<head><meta http-equiv=\"content-type\""
			"content=\"text/html;charset=utf-8\">"
			"</head><h1>404！您的页面已经进入外太空</h1>";//html标签，代表一级标题
		std::stringstream ss;
		ss << resp->body.size();
		std::string size;
		ss >> size;
		resp->header["Content-Length"] = size;//字符串转为数字
		return 0;
	}
	//1.通过Request 中的url_path字段，计算出文件在磁盘上的路径是什么
	//例如 url_path/index.html，想要得到磁盘上的文件就是./wwwroot/index.html
	//例如url_path/image/1.jpg
	//注意：./wwwroot 是我们此处约定的根目录，可以根据自己的喜好约定名字
	//2.打开文件，将文件中的所有内容读取出来放到body中
	int HttpServer::ProcessStaticFile(Context*context)
	{
		const Request& req = context->req;
		Response*resp = &context->resp;
		//1.获取到静态文件的完整路径
		std::string file_path;
		GetFilePath(req.url_path, &file_path);
		//2.打开并读取完整的文件
		int ret = FileUtil::ReadAll(file_path, &resp->body);
		if (ret < 0)
		{
			LOG(ERROR) << "ReadAll error! file_path=" << file_path << "\n";
			return -1;
		}
		return 0;
	}
	//功能的实现主要就是由CGI进行实现的，锁一在后序需要进行改善项目的时候，就是通过修改这部分的代码
	int HttpServer::ProcessCGI(Context*context)
	{
		const Request&req = context->req;
		Response*resp = &context->resp;
		//1.创建一对匿名管道（父子进城要双向通信）
		int fd1[2], fd2[2];
		pipe(fd1);
		pipe(fd2);
		int father_write = fd1[1];//父进程写
		int child_read = fd1[0];
		int child_write = fd2[1];
		int father_read = fd2[0];
		//2.设置环境变量
		//a)METHOD请求方法
		std::string env = "REQUEST_METHOD=" + req.method;
		putenv(const_cast<char*>(env.c_str()));
		//b)GET方法 QUERY_STRING请求的参数
		if (req.method == "GET")
		{
			env = "QUERY_STRING=" + req.query_string;
			putenv(const_cast<char*>(env.c_str()));
		}
		else if (req.method == "POST")
			//c)POST方法，就设置CONTENT_LENGTH
		{
			//Header::const_iterator 
			auto pos = req.header.find("Content-Length");//用auto自动推导类型
			env = "CONTENT_LENGTH=" + pos->second;
			putenv(const_cast<char*>(env.c_str()));
		}
		pid_t ret = fork();
		if (ret < 0)
		{
			perror("fork");
			goto END;
		}
		if (ret > 0)
		{
			//父进程开始先把子进程的文件描述符关闭
			close(child_read);
			close(child_write);
			//3.fork，父进程流程
			//a)如果是POST请求，父进程就要把body写入到管道中
			if (req.method == "POST"){
				write(father_write, req.body.c_str(), req.body.size());
			}
			//b)阻塞式的读取管道，尝试把子进程的结果读取出来，并且放到Response对象中
			FileUtil::ReadAll(father_read, &resp->cgi_resp);
			//c)对子进程进行进程等待（编僵尸进程）      
			wait(NULL);
		}
		else
		{
			close(father_read);
			close(father_write);
			//4.fork，子进程流程
			//a)把标准输入和标准输出进行重定向
			//打到新号码，new是old的拷贝 标准输入输出是new
			dup2(child_read, 0);
			dup2(child_write, 1);
			//b)先要获取到要替换的可执行文件是哪个（通过url_path中获取）
			std::string file_path;
			GetFilePath(req.url_path, &file_path);
			//c)进行进程的程序替换
			execl(file_path.c_str(), file_path.c_str(), NULL);
			//d)由CGI 可执行程序完成动态页面的计算，并且写回数据到管道中
			//这部分逻辑，我们需要放到另外的单独的文件中实现并且根据该文件编译生成新的CGI可执行程序
		}
	END:
		//TODO 统一处理收尾工作
		close(father_read);
		close(father_write);
		close(child_read);
		close(child_write);
		return 0;
	}

	//通过url_path找到对应的文件路径
	//例如请求url可能是 http://192.168.43.110:9090/ 
	//这种情况下 url_path是/
	//此时请求等价于 /index.html 
	//例如请求url可能是 http://192.168.43.110:9090/image/
	//这种情况下url_path是/
	//如果url_path指向的是一个目录，就尝试在这个目录下访问一个叫做index.html的文件（这个策略也是我们的一种简单约定） 
	void HttpServer::GetFilePath(const std::string&url_path, std::string*file_path)
	{
		*file_path = "./wwwroot" + url_path;
		//判断一个路径是普通文件还是目录文件
		//1.Linux的stat函数
		//2.通过 boost filesystem模块来进行判定
		//如果当前文件是一个目录，就可以进行一个文件名拼接，拼接上index.html
		if (FileUtil::IsDir(*file_path))
		{
			//1./image/
			//2./image
			if (file_path->back() != '/')
			{
				file_path->push_back('/');
			}
			(*file_path) += "index.html";
		}
		return;
	}
	int HttpServer::ParseFirstLine(const std::string&first_line, std::string*method, std::string*url)
	{
		std::vector<std::string> tokens;
		StringUtil::Split(first_line, " ", &tokens);//字符串的切分
		if (tokens.size() != 3)
		{
			//首行格式不对p
			LOG(ERROR) << "ParseFirstLine error! split error! first_line=" << first_line << "\n";
			return -1;
		}
		//版本号中不包含HTTP关键字，也认为出错
		if (tokens[2].find("HTTP") == std::string::npos)
		{
			//首行的格式不对，版本号中包含HTTP关键字
			LOG(ERROR) << "ParseFirstLine error! version error! first_line=" << first_line << "\n";
			return -1;
		}
		*method = tokens[0];
		*url = tokens[1];
		return 0;
	}
	//https//www.baidu.com/s?ie=utf-8&f=8&rsv_bp=0&rsv_idx=1&tn=baidu....
	//解析一个标准的url，其实比较复杂，核心思路是以？作为分割，从？左边来 查找url_path，从？右边来查找query_string
	//我们此处只是实现一个简化的版本，只考虑不包含域名和协议的情况
	//例如：/s?ie=utf-8&rsv_bp=0&rsv_idx=1&tn=baidu&wd-%e9%b2%9c%e8%8a%b1
	//我们就单纯的以？作为分割，？左边的就是path，？右边的就是query_string
	int HttpServer::ParseUrl(const std::string&url, std::string*url_path, std::string*query_string)
	{
		size_t pos = url.find("?");
		if (pos == std::string::npos)
		{
			//没找到
			*url_path = url;
			*query_string = "";//可能并没有query_string访问百度首页
			return 0;
		}
		//找到了
		*url_path = url.substr(0, pos);
		*query_string = url.substr(pos + 1);//第二个参数不填写，就默认到字符串的结尾
		return 0;
	}
	int HttpServer::ParseHeader(const std::string&header_line, Header*header)
	{
		size_t pos = header_line.find(":");
		if (pos == std::string::npos)
		{
			LOG(ERROR) << "ParseHeader error! has no : header_line=" << header_line << "\n";
			return -1;
		}
		//这个header的格式还是不正切，没有value
		if (pos + 2 >= header_line.size())
		{
			LOG(ERROR) << "ParseHeader error! has no value! header_line=" << header_line << "\n";
			return -1;
		}
		(*header)[header_line.substr(0, pos)] = header_line.substr(pos + 2);
		return 0;
	}


	/////////////////////////////////////////////////////////////////////// 
	///以下为测试函数
	/////////////////////////////////////////////////////////////////////// 
	void HttpServer::PrintRequest(const Request&req)
	{
		LOG(DEBUG) << "Request:" << "\n" << req.method << " " << req.url << "\n" << req.url_path << " " << req.query_string << "\n";
		//C++11 的range based for auto 会自动推导出类型
		for (auto it : req.header)
			// for(Header::const_iterator it = req.header.begin(); it != req.header.end(); ++it)
		{
			LOG(DEBUG) << it.first << ":" << it.second << "\n";
		}
		LOG(DEBUG) << "\n";
		LOG(DEBUG) << req.body << "\n";
	}



}//end http_server 
