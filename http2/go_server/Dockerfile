FROM ubuntu:latest
WORKDIR /root
ENV PATH="$PATH:/usr/go/bin"

RUN apt-get -y update && apt-get install -y wget && \
    wget https://go.dev/dl/go1.20.3.linux-amd64.tar.gz && \ 
    tar -C /usr -xzf go1.20.3.linux-amd64.tar.gz 

COPY http2_server.go /root
RUN go mod init server && go mod tidy && go mod download -x

ENTRYPOINT ["go", "run", "http2_server.go"]