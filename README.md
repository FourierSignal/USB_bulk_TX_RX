# USB_bulk_TX_RX

BULK class Firmware driver template modified for custom class driver.
need to rename files for readability/clarity.

This project uses usbd rom stack  for handling usb protocol.

usbd-rom-stack is not open unlike it's older version LPCUSBlib.

LPCUSBlib has both host stack and device stack.
 
now LPCUSBlib is used as host stack and usbdromstack as device stack. 
this is my understanding from sample code provided by lpcopen.

