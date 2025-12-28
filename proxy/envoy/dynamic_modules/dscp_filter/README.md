start envoy,
    export ENVOY_DYNAMIC_MODULES_SEARCH_PATH=$PWD
    ./envoy --log-level debug -c envoy-dynamic-filter.yaml 

then, setup traffic using,
    ./go_server -ip=127.0.0.1 -port=1447
    ./http2_client -ip=127.0.0.1 -port=10000

or use curl instead of client, 
    curl --http2-prior-knowledge -H "x-dscp: 46" http://127.0.0.1:10000/example

tcpdump and check if envoy is setting dscp is response headers.