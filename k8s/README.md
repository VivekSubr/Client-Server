example-pod is a example of golang microserve which serves http api and writes to redis.

example-operator is an operator for example-pod.


-------------------------------------------------------------
Minikube
-------------------------------------------------------------

Minikube sets up a single node cluster that can be used for testing. 
Kind project is used to simulate multi-node cluster.

Regardless, to run it on WSL, need to enable virtualization ->

(in powershell admin)
dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart
dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart
bcdedit /set hypervisorlaunchtype auto

Install kubectl and minikube and configure minikube to use docker.
minikube config set driver docker