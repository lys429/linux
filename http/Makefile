.PHONY:all
all:server cgi_main

server:http_server.cc http_server_main.cc
		g++ $^ -o $@ -std=c++11 -lpthread -lboost_filesystem -lboost_system
cgi_main:cgi_main.cc
		g++ $^ -o $@ -std=c++11 -lpthread -lboost_filesystem -lboost_system
		cp cgi_main ./wwwroot/add

.PHONY:clean
clean:
	rm server cgi_main
