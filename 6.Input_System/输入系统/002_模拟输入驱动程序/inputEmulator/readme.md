## How to test
```
busybox mount -t nfs -o nolock,vers=2 192.168.1.123:/work/nfs_root /mnt

insmod input_emulator.ko

测试方法:
sendevent /dev/input/event5 1 2 1  // 1 2 1 : EV_KEY, KEY_1, down
sendevent /dev/input/event5 1 2 0  // 1 2 0 : EV_KEY, KEY_1, up
sendevent /dev/input/event5 0 0 0  // sync

sendevent /dev/input/event5 1 3 1 
sendevent /dev/input/event5 1 3 0 
sendevent /dev/input/event5 0 0 0 
```
