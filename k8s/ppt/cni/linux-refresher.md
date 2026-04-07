A refresher on linux networking concepts.

# Network interfaces
Every network connection appears as a interface, to view them,
```ip link show        # List all interfaces
ip addr show        # Show IP addresses```

Example in UC worker node,
```[root@subramaniamv-tk5-k8-node-1-o0dp7x-zy8li9n3p2ggrqg4 ~]# ip link show
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN mode DEFAULT group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
2: ens3: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc fq_codel state UP mode DEFAULT group default qlen 1000
    link/ether fa:16:3e:90:0b:75 brd ff:ff:ff:ff:ff:ff
    altname enp0s3
7: nodelocaldns: <BROADCAST,NOARP> mtu 1500 qdisc noop state DOWN mode DEFAULT group default
    link/ether b2:0f:78:a0:27:c9 brd ff:ff:ff:ff:ff:ff
8: cali50474d991ac@if2: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP mode DEFAULT group default qlen 1000
    link/ether ee:ee:ee:ee:ee:ee brd ff:ff:ff:ff:ff:ff link-netns cni-8a0e4d34-0dbb-3b04-e55d-fa7c3b8fd959
9: tunl0@NONE: <NOARP,UP,LOWER_UP> mtu 1480 qdisc noqueue state UNKNOWN mode DEFAULT group default qlen 1000
    link/ipip 0.0.0.0 brd 0.0.0.0
14: calie91595b471a@if3: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1480 qdisc noqueue state UP mode DEFAULT group default qlen 1000
    link/ether ee:ee:ee:ee:ee:ee brd ff:ff:ff:ff:ff:ff link-netns cni-0fdae05f-0947-f8a7-f8e1-ba2f054b153c
15: calicf1498c59fd@if3: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1480 qdisc noqueue state UP mode DEFAULT group default qlen 1000
    link/ether ee:ee:ee:ee:ee:ee brd ff:ff:ff:ff:ff:ff link-netns cni-f9f62f9c-c249-c3a9-4de1-082f31bde0e3
16: cali30567aa1efa@if3: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1480 qdisc noqueue state UP mode DEFAULT group default qlen 1000
    link/ether ee:ee:ee:ee:ee:ee brd ff:ff:ff:ff:ff:ff link-netns cni-3ca2cb05-527c-5b42-e030-c3c18a3329fd```
```

Common link types 
* eth0, enp3s0 - Physical Ethernet
* wlan0 - Wireless
* lo - Loopback (127.0.0.1)
* veth* - Virtual interfaces 
* tun/tap - Tunnel interfaces for VPNs


* How to to read it *
```
2: ens3: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc fq_codel state UP mode DEFAULT group default qlen 1000
│  │     │                                  │        │           │        │            │              │
│  │     │                                  │        │           │        │            │              └─> Queue length (1000 packets)
│  │     │                                  │        │           │        │            └─> Network group
│  │     │                                  │        │           │        └─> Operating mode
│  │     │                                  │        │           └─> Link state (UP = active)
│  │     │                                  │        └─> Queueing discipline (traffic control algorithm)
│  │     │                                  └─> Maximum Transmission Unit (packet size in bytes)
│  │     └─> Interface flags (capabilities/status)
│  └─> Interface name
└─> Interface index number
```

The calico interfaces are all veth 

```
8: cali50474d991ac@if2: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1480 qdisc noqueue state UP mode DEFAULT group default qlen 1000
   │                   │                                       │           │
   │                   │                                       │           └─> No queueing (fast path)
   │                   │                                       └─> MTU reduced for IP-in-IP
   │                   └─> @if2 = paired with interface #2 in pod namespace
   └─> Calico-generated name
```


# Routes
The routing table determines where packets go.

```
[root@subramaniamv-tk5-k8-node-1-o0dp7x-zy8li9n3p2ggrqg4 ~]# ip route
default via 10.144.128.1 dev ens3 proto static
10.144.128.0/21 dev ens3 proto kernel scope link src 10.144.134.117
192.168.41.64/26 via 10.144.134.118 dev tunl0 proto bird onlink
192.168.158.64/26 via 10.144.134.114 dev tunl0 proto bird onlink
blackhole 192.168.205.0/26 proto bird
192.168.205.1 dev cali50474d991ac scope link
192.168.205.4 dev calie91595b471a scope link
192.168.205.5 dev calicf1498c59fd scope link
192.168.205.6 dev cali30567aa1efa scope link
192.168.205.7 dev cali3b831decba5 scope link
192.168.205.8 dev caliac920608da2 scope link
192.168.205.9 dev cali4e68eccbf9c scope link
192.168.205.10 dev cali1ed4df5072d scope link
192.168.205.11 dev cali9bc6831d38b scope link
192.168.205.12 dev cali82a27cdcf53 scope link
192.168.205.13 dev calicc965f158d2 scope link
192.168.205.14 dev cali18fbf4f5a45 scope link
192.168.205.15 dev cali4f0a0f30f40 scope link
192.168.205.16 dev cali883927030c1 scope link
192.168.205.17 dev calie7d5b1e36ab scope link
192.168.205.18 dev cali52748080bbd scope link
192.168.205.19 dev cali21562ddabc0 scope link
192.168.205.20 dev cali1e1d1b8df92 scope link
192.168.205.21 dev cali9d2f3b17584 scope link
192.168.212.0/26 via 10.144.134.119 dev tunl0 proto bird onlink
```

calico here has mapped all pod ips to a veth interface. 

Pod Network Namespace              Host Network Namespace
┌─────────────────────┐           ┌────────────────────────┐
│                     │           │                         │
│   ┌──────────┐      │           │  ┌──────────────────┐   │
│   │   eth0   │◄─────┼───────────┼─►│cali50474d991ac   │   │
│   │  (veth)  │      │  Virtual  │  │     (veth)       │   │
│   └──────────┘      │   cable   │  └─────────┬────────┘   │
│                     │           │            │            │
│  10.244.1.5/32      │           │            ▼            │
│                     │           │      Routing/iptables   │
└─────────────────────┘           │            │            │
                                  │            ▼            │
                                  │         ens3 (physical) │
                                  └────────────────────────┘

# Bridges
Bridges are like veth, but lower level (L2). Calico is purely L3, it does *not* use bridges. 

Bridges are lower level and hence has to go deeper - redirection is to MAC table level.

# Network Namespaces
A network namespace is an isolated copy of the network stack. Each namespace has its own.

# IpTables
```
[root@subramaniamv-tk5-k8-node-1-o0dp7x-zy8li9n3p2ggrqg4 ~]# ip netns list
cni-9452c888-9079-1c86-2460-8ab02f1139a0 (id: 9)
cni-e2ca6c00-ab31-3d4a-ebbb-89cffc82b275 (id: 8)
cni-53a3733b-2935-5f92-029d-68330ad77d6d (id: 7)
cni-d7fda231-098f-6d65-6bc6-3492bbd50cca (id: 6)
cni-484b767d-70c4-5bbc-255a-3b2b42b184be (id: 5)
cni-c2384efe-b074-da12-ac0d-01257dabfc23 (id: 4)
cni-79f36a47-22e3-07f0-f596-a3ae891c5b7e (id: 3)
cni-010609df-2c22-e5a3-4991-a0499307319e (id: 2)
cni-5f54b7b4-015d-66ff-95b3-19b432554559 (id: 1)
cni-1dc36750-ab34-884c-5262-84180ae79d34 (id: 0)
```

These netns were created by calico to wire up pods to the veth interfaces. Each cni-* namespace has its own full network stack:

    lo (loopback)
    eth0 (pod interface)
    Pod IP
    Routing table
    iptables rules
    Ports & sockets

```
[root@subramaniamv-tk5-k8-node-1-o0dp7x-zy8li9n3p2ggrqg4 ~]# ip netns exec cni-1dc36750-ab34-884c-5262-84180ae79d34 ip link
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN mode DEFAULT group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
2: eth0@if8: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP mode DEFAULT group default qlen 1000
    link/ether fe:cc:aa:af:f1:4a brd ff:ff:ff:ff:ff:ff link-netnsid 0
3: tunl0@NONE: <NOARP> mtu 1480 qdisc noop state DOWN mode DEFAULT group default qlen 1000
    link/ipip 0.0.0.0 brd 0.0.0.0
```

Here, eth0@if8 is the important part it means - “eth0 is one end of a veth pair, and the other end has interface index 8 in another namespace.”

So, from output of 'ip link show', eth0 in pod maps to 'cali344ea0df667@if2' veth interface.

```
8: cali344ea0df667@if2: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP mode DEFAULT group default qlen 1000
    link/ether ee:ee:ee:ee:ee:ee brd ff:ff:ff:ff:ff:ff link-netns cni-1dc36750-ab34-884c-5262-84180ae79d34
```

All calico veth interfaces link to calico's tunnel interface, tunl0

```
3: tunl0@NONE: <NOARP> mtu 1480 qdisc noop state DOWN mode DEFAULT group default qlen 1000
    link/ipip 0.0.0.0 brd 0.0.0.0
```

Note the @NONE --> it means this tunnel is not linked to any other interface, it sends to actual NIC.

[ Pod ]
  eth0
   │   (veth)
   ▼
[ Node ]
  cali344ea0df667
   │
   │  (routing lookup)
   │
   ├─ if destination pod is LOCAL:
   │      → another cali* interface
   │
   └─ if destination pod is REMOTE:
          → tunl0
              (IP-in-IP encapsulation)
              ↓
           ens3 (physical NIC)

