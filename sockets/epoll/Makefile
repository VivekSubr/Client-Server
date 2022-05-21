build:
	make -C go_client build
	g++ -g -o server.exe epoll_server.cc

clean:
	make -C go_client clean
	rm -fr *.o *.exe