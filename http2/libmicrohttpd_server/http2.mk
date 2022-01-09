build:
	g++ -o server.exe server.cc -lmicrohttpd

clean:
	rm -f server.exe *.o