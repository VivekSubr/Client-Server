# Filesystems 
A filesystem is the layer that answers: "given a block device full of raw bytes, how do I organize, name, find, and manage data?" It provides the abstraction between physical storage (spinning disks, SSDs, NVMe) and the files/directories you work with.

## Linux Storage Stack
```
Application (open, read, write, stat)
        ↓
   VFS (Virtual Filesystem Switch)
        ↓
  Filesystem driver (ext4, xfs, btrfs...)
        ↓
   Page Cache / Buffer Cache
        ↓
   Block Layer (I/O scheduler, request queue)
        ↓
   Device Driver (SCSI, NVMe, SATA)
        ↓
   Physical Storage
```