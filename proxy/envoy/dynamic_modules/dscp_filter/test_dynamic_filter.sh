set -x

function kill_proc() {
    killall envoy
    killall http2_client
    killall server
}

trap `kill_proc` SIGINT

SERVER_IP=127.0.0.1
CLIENT_IP=127.0.0.1
SERVER_PORT=1447
CLIENT_PORT=10000
GO_SERVER=go_server/server
GO_CLIENT=go_client/http2_client

export ENVOY_DYNAMIC_MODULES_SEARCH_PATH=$PWD
./envoy --log-level debug -c envoy-dynamic-filter.yaml --log-path $PWD/envoy.log &

#Run http2 server and client, both should refer to envoy proxy
$GO_SERVER -ip=$SERVER_IP -port=$SERVER_PORT &> server.log &
$GO_CLIENT -ip=$CLIENT_IP -port=$CLIENT_PORT &> client.log &
