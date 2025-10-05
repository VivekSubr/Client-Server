--------------------------------------------
INIT
--------------------------------------------
Example operator using kubebuilder. Initiliazed by commands,
go mod init example.com/m/v2
kubebuilder init --domain=example.com

Then,
kubebuilder create api --group cache --version v1alpha1 --kind Redis

edit internal/controller/redis_controller 

'make help'

---------------------------------------------
TEST
---------------------------------------------

This uses KinD to simulate kubernetes cluster and run e2e tests. KinD needs docker to be running.
