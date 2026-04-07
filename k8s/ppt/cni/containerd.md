Kubernetes
  ↓
kubelet
  ↓
CRI (Container Runtime Interface)
  ↓
containerd
  ↓
runc (OCI runtime)
  ↓
Linux kernel (namespaces, cgroups)


* runc is a linux userspace component that implements the OCI, Open Container Interface, to let docker, containerd to use kernel features (namespace/cgroups) that underpin containers.