# 3DPrinterCam
Low budget and simple ESP32-CAM 3DPrinter Camera to make time-lapse videos.

I have recently purchased a Prusa MK4 printer. It does not have a camera. I do not wish to bring in OctoPrint given the printer is already WIFI enabled and has an extra USB port. The code and hardware for this project should work with any printer vendor.

The ESP32-CAM is a very capable self-contained Arduino WIFI-enabled camera. The camera may be purchased from many vendors. It runs about 10 US$, including a USB-TTL Serial (USB) Adapter Module. There are many excellent YouTube videos to take you through setting up the camera for development.

The MK4 does not provide power from its USB-C port. If your printer will not supply 5V then you will need a USB Y Cable designed to supply supplimental power from the extra plug. There may be other ways to pull the power from the MK4 board.

You will need to modify your printer slicer settings so the printer can command the camera. There may be ideal GCODE pause commands.
- `M118 cam_begin` to begin gcode
- `M118 cam_frame` to layer change gcode
- `M118 cam_end` to end gcode before finalize
- `M118 cam_photo` to end gcode after finalize

You will need to copy MyConfig.hh to MyConfig.h and make any hardware declaration changes you need.

## Status:

Much of the code is cribbed from: [ESP32-CAM_MJPEG2SD](https://github.com/s60sc/ESP32-CAM_MJPEG2SD)
That project does a lot more than what we need and not exactly what we need. Despite being a global-variable mess, it is an excellent source.

I have not yet run an end to end time-lapse. Creating a timelapse is possible using the Arduino IDE Serial Monitor and curl/browser. The Serial read code does not cooperate well when the serial bus is flooded with data; needs to be reimplemented.

## Images
![Hardware](Hardware.jpeg)

## 3D Printed Components
[Self Promotion](https://www.printables.com/@Jove/models)

## My shopping list:
[ESP32-Cam](https://www.amazon.com/dp/B0948ZFTQZ?psc=1&ref=ppx_yo2ov_dt_b_product_details)\
Any micro SD Card\
[Power Adaptor](https://www.amazon.com/dp/B003HHK576?psc=1&ref=ppx_yo2ov_dt_b_product_details)\
[USB-A to USB-C](https://www.amazon.com/dp/B09SZ5NHF4?psc=1&ref=ppx_yo2ov_dt_b_product_details)\
[USB-Mini to USB-Micro](https://www.amazon.com/dp/B08FRNCQXP?psc=1&ref=ppx_yo2ov_dt_b_product_details)
