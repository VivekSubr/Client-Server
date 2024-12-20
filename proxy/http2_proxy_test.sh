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
GO_SERVER=go_server/server
GO_CLIENT=go_client/http2_client
ADDL_SERVER_FLAGS=
ADDL_CLIENT_FLAGS=

if [[ "$1" == "reverse" ]]; then
    SERVER_PORT=10000
    CLIENT_PORT=1447
    ENVOY_YAML=envoy-as-reverse-proxy.yaml
elif [[ "$1" == "L7" ]]; then
    ENVOY_YAML=envoy-L7.yaml
elif [[ "$1" == "tls" ]]; then
    ENVOY_YAML=envoy-tls.yaml
    GO_SERVER=tls/server/tlsServer
    GO_CLIENT=tls/client/tlsClient
    ADDL_CLIENT_FLAGS=-crt=tls/server/publickey.cer
    ADDL_SERVER_FLAGS="-crt=tls/server/publickey.cer -key=tls/server/private.pem"
else 
    ENVOY_YAML=envoy-as-proxy.yaml
fi

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

