# 设计概况
stm32F103+ESP8266+W5500+OLED  
FreeRTOS（对堆栈合理分配）  
OneNET远程4路开关应用  
心知天气  
cjson解析（剩余RAM太小了，没有进行移植）  
uart1发送DMA  
uart2接收DMA（DMA相关代码有，但实测ESP8266与F103用AT通信时不适合用DMA。会导致单次通信的返回内容被拆分为多条，不好处理）  

**注意**：代码中主要使用`OLED`、`ESP8266`，`CH340`作为USB串口调试输出。`W5500`的电路、代码验证无误，但没有在此工程嵌入相关代码。  

# 2021.04.19 PCB
AD-PCB工程已上传，全3D，显示起来蛮漂亮，不过工程大了些。打样直接把`F103-IOT-IC.PcbDoc`发JLC。  
![image](https://github.com/ZhiliangMa/202004-F103-IOT-DEV_V1/blob/master/img/F103-IOT-DEV-TOP.jpg)
![image](https://github.com/ZhiliangMa/202004-F103-IOT-DEV_V1/blob/master/img/F103-IOT-DEV-BOTTOM.jpg)

# 电源
支持`Micro USB`、`DC插头`输入，电路设有电源自动切换，两者可同时插入，不用担心两者冲突。  
预留18650电池座、锂电保护及充电电路，可维持掉电后的几个小时续航。  

# 硬件Bug
此PCB的RTC不起振，已改用`网页HTML获取时间`+`FreeRTOS软件定时器`的方式，隔几分钟会自动校准。  


# 硬件资源
STM32F103C8T6  
W5500  
ESP-12S/F  
EMW3080  
USB/CAN  
485  
SPI-OLED/TFT  
<br> 
uart1 -> CH340 -> usb Debug  
uart2 -> ESP12/EMW3080  
uart3 -> RS485/uart3  
SPI1 -> OLED/TFT  
SPI2 -> W5500  

# 引脚分配
WIFI_TX -> UART2_RX -> PA3  
WIFI_RX -> UART2_TX -> PA2  
WIFI_RST -> PA4  
WIFI_EN -> 无，硬件默认始终使能  
<br>
OLED_SCK -> SPI1_SCK -> PA5  
OLED_MOSI-> SPI1_MOSI -> PA7  
OLED_RST-> PA6  
OLED_DC-> PB0  
OLED_CS-> PB1  
<br> 
SWITCH1 -> SCR1 -> PB8  
SWITCH2 -> SCR2 -> PB5  
SWITCH3 -> JK1 -> PB4  
SWITCH4 -> JK2 -> PB3  
<br>
USB_Debug -> UART1_TX -> PA9  
USB_Debug -> UART1_RX -> PA10  
<br>
RS485_TX -> UART3_RX -> PB11  
RS485_RX -> UART3_TX -> PB10  
RS485_RD -> PB2  
