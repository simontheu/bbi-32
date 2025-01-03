# BBI-32 CLI App
BBI-32 GPS locked clock source config

## Prerequisites ##

gcc


## Build Instructions Using GCC ##

For the frequency setting and status utility:
```
    gcc -o bbi-32 bbi-32.c
```

## Finding Your Device ##

1. List hidraw devices:
```
    ls -l /dev/hidraw*

    crw------- 1 root root 246, 0 12. Aug 09:31 /dev/hidraw0
    crw------- 1 root root 246, 1 22. Aug 16:22 /dev/hidraw1
```

2. Check hidraw devices until you find the correct device
```
    cat /sys/class/hidraw/hidraw1/device/uevent

    DRIVER=hid-generic
    HID_ID=0003:00001DD2:00002443
    HID_NAME=Leo Bodnar Electronics LBE-1420 GPS Locked Clock Source
    HID_PHYS=usb-0000:02:00.0-2.1/input2
    HID_UNIQ=0673ED0E4101
    MODALIAS=hid:b0003g0001v00001DD2p00002443
```
3. Use the hidraw device as the first argument with the utility, eg
```
    ./bbi-32 /dev/hidraw1 --enc 11 type 1
```
