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

./envoy --log-level trace -c $PWD/$ENVOY_YAML --log-path $PWD/envoy.log &

./go_server/server $SERVER_IP $SERVER_PORT &> server.log
./go_client/http2_client $CLIENT_IP $CLIENT_PORT &> client.log

