### 硬件看门狗驱动

1. Please clone the this code, and put this package to OpenWrt [SDK](https://github.com/gl-inet/sdk)

2. Choose kmod-gl-hw-wdt package in make menuconfig
```
    Kernel modules  --->
        Other modules  ---> 
            <*> kmod-gl-hw-wdt............................... gl hardware watchdog driver
```

3. Compile it

4. How to use

When we add this package, the watchdog will autu run, we can use rmmod command to stop watchdog

```
    Open the watchdog command: echo 1 >/sys/class/gpio/gpio12/value
    Close the watchdog command: echo 0 >/sys/class/gpio/gpio12/value
```
