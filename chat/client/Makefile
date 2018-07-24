.PHONY:all 
	all:test_win test_client chat_client 
chat_client::chat_cl.cc window.cc chat_cl_main.cc 
		g++ -o $@ $^ -lpthread -ljsoncpp -lboost_filesystem -lboost_system -lncursesw -std=c++11
test_win:window.cc 
		g++ -o $@ $^ -lpthread -lncursesw -std=c++11 -DTEST_WINDOW -lboost_filesystem -lboost_system 
test_client:chat_cl.cc 
		g++ -o $@ $^ -lpthread -lboost_filesystem -lboost_system -ljsoncpp -DTEST_CHAT_CLIENT
.PHONY:clean 
clean:
		rm test_client test_win chat_client
