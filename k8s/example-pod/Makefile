build:
	go build -o example *.go

generate:
	go generate main.go 

mod:
	go mod download
	go mod tidy

clean:
	go clean
	rm -f example server-gen.go client-gen.go go.sum