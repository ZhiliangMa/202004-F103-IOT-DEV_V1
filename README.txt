��Ƹſ���
FreeRTOS���Զ�ջ������䣩
cjson����
uart1����DMA
uart2����DMA
uart3����DMA


Ӳ����Դ��
STM32F103C8T6
W5500
ESP-12S/F
EMW3080
USB/CAN
485
SPI-OLED/TFT

uart1 -> CH340 -> usb Debug
uart2 -> ESP12/EMW3080
uart3 -> RS485/uart3
SPI1 -> OLED/TFT
SPI2 -> W5500

���ŷ��䣺
WIFI_TX -> UART2_RX -> PA3
WIFI_RX -> UART2_TX -> PA2
WIFI_RST -> PA4
WIFI_EN -> �ޣ�Ӳ��Ĭ��ʼ��ʹ��

SWITCH1 -> SCR1 -> PB8
SWITCH2 -> SCR2 -> PB5
SWITCH3 -> JK1 -> PB4
SWITCH4 -> JK2 -> PB3

USB_Debug -> UART1_TX -> PA9
USB_Debug -> UART1_RX -> PA10

RS485_TX -> UART3_RX -> PB11
RS485_RX -> UART3_TX -> PB10
RS485_RD -> PB2