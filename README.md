### 硬件看门狗驱动

修改 src/gl_hw_wdt.c中的喂狗gpio宏定义为喂狗gpio，如喂狗gpio为1
```
   #define HW_WDI	1
```