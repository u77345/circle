#
# Makefile
#

CIRCLEHOME = ../..

OBJS	= hd44780device.o st7789display.o chardevice.o ssd1306device.o ili9341.o

libdisplay.a: $(OBJS)
	@echo "  AR    $@"
	@rm -f $@
	@$(AR) cr $@ $(OBJS)

include $(CIRCLEHOME)/Rules.mk

-include $(DEPS)
