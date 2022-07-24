set -x

function kill_proc() {
    killall envoy
    killall client
    killall http2-example
}

trap `kill_proc` SIGINT

SERVER_IP=127.0.0.1
CLIENT_IP=127.0.0.1
SERVER_PORT=1447
CLIENT_PORT=10000

if [[ "$1" == "reverse" ]]; then
    SERVER_PORT=10000
    CLIENT_PORT=1447
    ENVOY_YAML=envoy-as-reverse-proxy.yaml
elif [[ "$1" == "L7" ]]; then
    ENVOY_YAML=envoy-L7.yaml
elif [[ "$1" == "tls" ]]; then
    ENVOY_YAML=envoy-tls.yaml
else 
    ENVOY_YAML=envoy-as-proxy.yaml
fi

#Run envoy proxy
make clean build
envoy --log-level debug -c envoy/$ENVOY_YAML --log-path $PWD/envoy.log &

#Run http2 server and client, both should refer to envoy proxy
./go_server/http2-example -ip=$SERVER_IP -port=$SERVER_PORT &> server.log &
./go_client/client -ip=$CLIENT_IP -port=$CLIENT_PORT &> client.log &

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

