* ./run.sh to start up jaeger and otel collector
* make and start go_server
* make and ./example in build folder, type 'http' to send a message
* Look at traces in http://localhost:16686


----------------------------------------------------------------
BUILD
----------------------------------------------------------------
Opentelemetry has dependency on abseil (if below C++20), googletest, protobuf and grpc.

Use the ci scripts to set these up,
    sudo -E ./ci/setup_ci_environment.sh
    sudo -E ./ci/install_protobuf.sh 
    sudo ./ci/setup_grpc.sh

* To see more info, do make VERBOSE=1 instead of make in build folder.