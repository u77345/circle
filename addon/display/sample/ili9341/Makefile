#
# Makefile
#

CIRCLEHOME = ../../../..

OBJS	= main.o kernel.o

LIBS= $(CIRCLEHOME)/addon/display/libdisplay.a \
	  $(CIRCLEHOME)/addon/gpio/libgpio.a \
	  $(CIRCLEHOME)/lib/usb/libusb.a \
	  $(CIRCLEHOME)/lib/input/libinput.a \
	  $(CIRCLEHOME)/lib/fs/libfs.a \
	  $(CIRCLEHOME)/lib/libcircle.a \
	  $(CIRCLEHOME)/addon/sensor/libsensor.a \
	  $(CIRCLEHOME)/addon/utils/lib_my_utils.a 


include $(CIRCLEHOME)/Rules.mk

-include $(DEPS)
