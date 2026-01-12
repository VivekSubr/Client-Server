start envoy,
    export ENVOY_DYNAMIC_MODULES_SEARCH_PATH=$PWD
    ./envoy --log-level debug -c envoy-dynamic-filter.yaml 

then, setup traffic using,
    ./go_server -ip=127.0.0.1 -port=1447
    ./http2_client -ip=127.0.0.1 -port=10000

or use curl instead of client, 
    curl --http2-prior-knowledge -H "x-dscp: 46" http://127.0.0.1:10000/example


Check tcpdump for if dscp is set,  sudo tcpdump -i lo -v 'port 1447' --> for upstream
                                   
Should see something like, 

04:29:28.602316 IP (tos 0xb8, ttl 64, id 22637, offset 0, flags [DF], proto TCP (6), length 52)
    127.0.0.1.46436 > 127.0.0.1.1447: Flags [.], cksum 0xfe28 (incorrect -> 0xe431), seq 1, ack 1, win 512, options [nop,nop,TS val 3944218569 ecr 3944218569], length 0

This shows dscp is set from envoy to 1447 (server), based on request header.

For response header, don't set in request, 
        curl --http2-prior-knowledge -H http://127.0.0.1:10000/example                            
                 
server should set in response, and capture like this,
    sudo sudo tcpdump -i lo -v -n 'port 10000'

And you should see something like,
13:35:09.281451 IP (tos 0x88, ttl 64, id 49366, offset 0, flags [DF], proto TCP (6), length 52)
    127.0.0.1.10000 > 127.0.0.1.43208: Flags [F.], cksum 0xfe28 (incorrect -> 0xcaaa), seq 202, ack 122, win 512, options [nop,nop,TS val 407378800 ecr 407378800], length 0