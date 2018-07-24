chat_server:chat_ser.cc chat_ser_main.cc
		g++ -o $@ $^ -std=c++0x -lpthread -lboost_filesystem -lboost_system -ljsoncpp
.PHONY:clean
	clean:
		rm chat_server
