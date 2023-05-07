A Simple, test http2 server. (go mod init server + go mod tidy to start using)

Can be built as plain exe (make build) or docker image (make docker). 

k8s folder has resources that can be used with this docker image in k8s... in case of minikube, this image must be local to the 
minikube VM for usage.
So,
minikube start (docker should be running... dockerd might not work, docker desktop prefered)
eval $(minikube -p minikube docker-env) ... this switches context to minikube VM
now make docker and the image will be created in minikube VM, and k8s can use (kc apply -f deploy.yaml)