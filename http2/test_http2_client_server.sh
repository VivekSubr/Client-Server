make -f http2.mk clean build

./libmicrohttpd_server/server.exe &> server.log
./go_client/client -ip=127.0.0.1 -port=8888 &> client.log &

killall client
killall server