Calico-IPAM Explained

IPAM = **IP Address Management** — Calico's system for assigning pod IP addresses.

## Core concept

Instead of a central IPAM server, Calico uses a **distributed block allocation** model:

1. **Each node gets a block** (CIDR range) from the cluster's pod CIDR
2. **Node allocates from its local block** — fast, no coordination needed
3. **Blocks stored in Kubernetes API** — acts as the distributed datastore

## Example allocation

Cluster pod CIDR: `10.244.0.0/16`
```
Node 1 gets block: 10.244.0.0/26   (64 IPs: .0 to .63)
Node 2 gets block: 10.244.0.64/26  (64 IPs: .64 to .127)
Node 3 gets block: 10.244.0.128/26 (64 IPs: .128 to .191)
```

When containerd on Node 1 creates a pod:
- Calico-IPAM picks next free IP from `10.244.0.0/26`
- Let's say it assigns `10.244.0.15/32`
- Marks it as used in the block allocation bitmap
- Returns to CNI plugin

## How it works under the hood

### 1. Block allocation storage

Calico stores IPAM state as Kubernetes CRDs:
```bash
kubectl get ipamblocks.crd.projectcalico.org -A
```

Each block is a CR like:
```yaml
apiVersion: crd.projectcalico.org/v1
kind: IPAMBlock
metadata:
  name: 10-244-0-0-26
spec:
  cidr: 10.244.0.0/26
  affinity: "host:subramaniamv-tk5-k8-node-1-o0dp7x-zy8li9n3p2ggrqg4"
  allocations: [0, 1, 2, null, null, ...] # bitmap of used IPs
  attributes:
    - handle_id: "pod-abc123"
      secondary: {...}
```

### 2. Allocation process

When `/opt/cni/bin/calico` runs:
```
1. Read node's assigned block from K8s API
   ↓
2. Find first free IP in the bitmap
   ↓
3. Update IPAMBlock CR (atomic K8s update)
   ↓
4. Return IP to CNI plugin
```

### 3. Block size configuration

Default block size is `/26` (64 IPs per node). Configurable via:
```yaml
apiVersion: projectcalico.org/v3
kind: IPAMConfig
metadata:
  name: default
spec:
  autoAllocateBlocks: true
  strictAffinity: false
  maxBlocksPerHost: 5  # Node can have up to 5 blocks
```

## Why this design?

**Pros:**
- **Fast** — no central bottleneck, node allocates locally
- **Scalable** — each node is independent
- **Reliable** — uses K8s API as distributed store (etcd backing)

**Cons:**
- **IP fragmentation** — if a node is deleted, its block is wasted
- **Block exhaustion** — small nodes with many pods may need multiple blocks

## Check your node's IPAM blocks
```bash
# See all IPAM blocks
kubectl get ipamblocks.crd.projectcalico.org -o wide

# See IPs allocated on your specific node
calicoctl ipam show --show-blocks
```

## Explanation of example 'calico-ipam.yaml'

```

kind: IPAMBlock
name: 192-168-41-64-26
cidr: 192.168.41.64/26
affinity: host:subramaniamv-tk5-k8-node-2-...
```

It means 192.168.41.64/26 is assigned to node-2