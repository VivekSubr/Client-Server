set -x

function kill_proc() {
    killall envoy
    killall sock_server.exe
    killall sock_client.exe
}

trap `kill_proc` SIGINT

SERVER_IP=127.0.0.1
CLIENT_IP=127.0.0.1
SERVER_PORT=1447
CLIENT_PORT=10000
ENVOY_YAML=envoy-dscp-wasm.yaml

pushd sockets 
make clean build
popd 

envoy --log-level debug -c $PWD/$ENVOY_YAML --log-path $PWD/envoy.log &

./sockets/sock_server.exe $SERVER_IP $SERVER_PORT &> server.log
./sockets/sock_client.exe $CLIENT_IP $CLIENT_PORT &> client.log

