start envoy,
    export ENVOY_DYNAMIC_MODULES_SEARCH_PATH=$PWD
    ./envoy --log-level debug -c envoy-dynamic-filter.yaml 

then, setup traffic using,
    ./go_server -ip=127.0.0.1 -port=1447
    ./http2_client -ip=127.0.0.1 -port=10000

or use curl instead of client, 
    curl --http2-prior-knowledge -H "x-dscp: 46" http://127.0.0.1:10000/example


Check tcpdump for if dscp is set,  sudo tcpdump -i lo -v 'port 1447'
(Note, -i lo may not capture dscp packets!)