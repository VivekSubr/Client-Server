## What is this?
A set of docs explaining Container Network Interfaces (CNI)

### What CNI does 

1. Create veth pair (virtual ethernet)
2. Move one end to container netns
3. Assign IP from IPAM plugin
4. Set up routes and gateway
5. Configure DNS if specified
6. Return result to runtime

```mermaid
flowchart TD
    A[Container runtimeCreates container] --> B[Runtime calls CNIPasses network namespace]
    B --> C[CNI pluginCreates veth pairsAssigns IP addresses]
    C --> D[Returns to runtimeNetwork configuration]
    D --> E[Container readyNetwork connectivity live]
    
    style A fill:#E6F1FB,stroke:#185FA5
    style B fill:#EEEDFE,stroke:#534AB7
    style C fill:#E1F5EE,stroke:#0F6E56
    style D fill:#FAEEDA,stroke:#854F0B
    style E fill:#EAF3DE,stroke:#3B6D11
```

CNI is called by containerd when bringup up the containers in a pod.

Basically,

containerd → executes /opt/cni/bin/calico → returns IP config → containerd stores it
