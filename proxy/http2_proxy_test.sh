function kill_proc() {
    killall envoy
    killall client
    killall http2-example
}

trap `kill_proc` SIGINT

#Run envoy proxy
make clean build
envoy -c envoy/envoy-as-proxy.yaml &> envoy.log & 

#Run http2 server and client, both should refer to envoy proxy
./go_server/http2-example -ip=127.0.0.1 -port=1447 &> server.log &
./go_client/client -ip=127.0.0.1 -port=10000 &> client.log &

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

