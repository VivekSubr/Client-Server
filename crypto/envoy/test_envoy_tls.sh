set -x

function kill_proc() {
    killall envoy
    killall http2_client
    killall server
    killall tlsServer
    killall tlsClient
}

trap `kill_proc` SIGINT

SERVER_IP=127.0.0.1
CLIENT_IP=127.0.0.1
SERVER_PORT=1447
CLIENT_PORT=10000
ENVOY_YAML=envoy-config.yaml
GO_SERVER=../server/tlsServer
GO_CLIENT=../client/tlsClient
ADDL_CLIENT_FLAGS="-key=/ca/tls-key"
ADDL_SERVER_FLAGS="-crt=/ca/tls-crt -key=/ca/tls-key"

#Run envoy proxy
make clean build
envoy --log-level debug -c envoy/$ENVOY_YAML --log-path $PWD/envoy.log &

#Run http2 server and client, both should refer to envoy proxy
$GO_SERVER -ip=$SERVER_IP -port=$SERVER_PORT $ADDL_SERVER_FLAGS &> server.log &
$GO_CLIENT -ip=$CLIENT_IP -port=$CLIENT_PORT $ADDL_CLIENT_FLAGS &> client.log &

#verify communication
tail -f server.log | while read temp
do
    found=`echo $temp | grep "Replied"`
    echo $found
    if [[ "$found" == "Replied" ]]; then
        echo "test passed\n"
        break
    fi
done

#kill process
kill_proc