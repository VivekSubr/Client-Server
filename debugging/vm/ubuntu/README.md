WSL is often not good enough for testing, as it shares windows networking, filesystem (that's why windows files accessible via /mnt/c)

For ubuntu full VM, we can use *multipass*

```
multipass launch --name dev --cpus 2 --memory 4G --disk 20G
multipass shell dev        # drops you into the VM
multipass list             # see all VMs
multipass stop dev
multipass delete dev && multipass purge
```

It can also mount windows directories:
```multipass mount C:\Users\vivek\projects dev:/home/ubuntu/projects```

