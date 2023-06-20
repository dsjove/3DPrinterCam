# 3DPrinterCam
ESP32-CAM 3DPrinter Camera

I have recently purchased a Prusa MK4 printer. It does not have a camera. I do not wish to bring in OctoPrint given the printer is already WIFI enabled and has an extra USB port. The code and hardware for this project should work with any printer vendor.

The ESP32-CAM is a very capable self-contained Arduino WIFI-enabled camera. The camera may be purchased from many vendors. It runs about 10 US$, including a USB-TTL Serial (USB) Adapter Module. There are many excellent YouTube videos to take you through setting up the camera for development.

The MK4 does not provide power from its USB-C port. If your printer will not supply 5V then you will need a USB Y Cable designed to supply supplimental power from the extra plug.

You will need to modify your printer slicer settings so the printer can command the camera
- `M118 snapbegin` to begin
- `M118 snapend` to end
- `M118 snaplayer` to layer change

I am cribbing a lot of logic from: [ESP32-CAM_MJPEG2SD](https://github.com/s60sc/ESP32-CAM_MJPEG2SD)
That project does a lot more than what we need and not exactly what we need.

## Code Status:

### PrinterCam.nio
TODO: serial reads for the commands are fine for protoype but should be more robust

### Globals.h/cpp camera_pins.h
Junk Drawer. This is where the hardware defines reside.

### Camera.h/cpp
TODO: expose camera settings and flash for external control

### Storage.h/cpp
TODO: test other things than just the SD Card
TODO: file listing for web

### AVI.h/cpp
TODO: max frames compare fails

### WifiConnection.h/cpp
Unused right now. Will enable wifi and time configuration.

### CamServer
We will want a web server that can serve the files, display last taken picture, and camera configuration.

![Hardware](Hardware.jpeg)













