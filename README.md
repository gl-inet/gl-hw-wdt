### 硬件看门狗驱动

1. 将工程clone到openwrt sdk的package目录

2. 修改 src/gl_hw_wdt.c中的喂狗gpio宏定义为喂狗gpio，如喂狗gpio为1
```
   #define HW_WDI	1
```

3. make menuconfig选上kmod-gl-hw-wdt
```
    Kernel modules  --->
        Other modules  ---> 
            <*> kmod-gl-hw-wdt............................... gl hardware watchdog driver
```

4. 编译

5. 使用

    选上此包后，喂狗程序会自动启动，可通过rmmod 移除模块来停止喂狗

```
    开启看门狗: echo 1 >/sys/class/gpio/gpio12/value
    关闭看门狗: echo 0 >/sys/class/gpio/gpio12/value
```