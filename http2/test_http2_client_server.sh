set -x

function kill_proc() {
    killall envoy
    killall client
    killall server
}

trap `kill_proc` SIGINT

make clean build

SERVER_IP=127.0.0.1
CLIENT_IP=127.0.0.1
SERVER_PORT=1447
CLIENT_PORT=10000
NGHTTP2_SERVER=nghttp2_client/client.exe
NGHTTP2_CLIENT=nghttp2_server/server.exe

#start proxy, server and client
envoy --log-level debug -c proxy/envoy-L7.yaml --log-path $PWD/envoy.log &

$NGHTTP2_SERVER $SERVER_IP $SERVER_PORT &> server.log &
$NGHTTP2_CLIENT $CLIENT_IP $CLIENT_PORT &> client.log &

