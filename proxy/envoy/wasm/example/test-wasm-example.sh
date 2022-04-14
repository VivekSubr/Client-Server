set -x

function kill_proc() {
    killall envoy
    killall client
    killall http2-example
}

#trap `kill_proc` SIGINT
kill_proc

SERVER_IP=127.0.0.1
CLIENT_IP=127.0.0.1
SERVER_PORT=1447
CLIENT_PORT=10000
ENVOY_YAML=envoy-wasm-example.yaml

WORK_DIR=../../../..
GO_CLIENT=$WORK_DIR/proxy/go_client/client
GO_SERVER=$WORK_DIR/proxy/go_server/http2-example

#build and set the wasm filter
make clean build
rm -f /home/vivek/example-filter.wasm && cp -f example-filter.wasm /home/vivek/.

#Run envoy proxy
envoy --log-level debug -c $ENVOY_YAML --log-path $PWD/envoy.log &

#Run http2 server and client, both should refer to envoy proxy
$GO_SERVER -ip=$SERVER_IP -port=$SERVER_PORT &> server.log &
$GO_CLIENT -ip=$CLIENT_IP -port=$CLIENT_PORT &> client.log &

sleep 1000
