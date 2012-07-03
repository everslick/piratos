-include ../Makefile.config

TARGET    = ../bin/$(PROGRAM)

KERNEL    = ../bin/$(PROGRAM).elf
IMAGE     = ../bin/$(PROGRAM).img
MAP       = ../bin/$(PROGRAM).map
LD_SCR    = kernel/boot/pi/linker.ld

ifeq ($(MAKECMDGOALS),pi)

PLATFORM  = pi

ARCH      = arm-linux-gnueabi
CC        = $(ARCH)-gcc
CPP       = $(ARCH)-g++
AS        = $(ARCH)-as
LD        = $(ARCH)-ld
AR        = $(ARCH)-ar
OBJCOPY   = $(ARCH)-objcopy

AFLAGS    = -march=arm -mcpu=arm1176jzf-s
CFLAGS    = -marm -mcpu=arm1176jzf-s

else

PLATFORM  = linux

CFLAGS    = `sdl-config --cflags`
LFLAGS    = `sdl-config --libs` -lrt -lpthread

endif

ifdef RELEASE

CFLAGS   += -O3 -DRELEASE
AFLAGS   +=

else

CFLAGS   += -O0 -g -Werror -DDEBUG
AFLAGS   += -g

endif

MODULES  += init                       \
            kernel/os                  \
            kernel/os/$(PLATFORM)      \
            kernel/boot                \
            kernel/boot/$(PLATFORM)    \
            kernel/hal/sd              \
            kernel/hal/sd/$(PLATFORM)  \
            kernel/hal/fb              \
            kernel/hal/fb/$(PLATFORM)  \
            kernel/hal/kbd             \
            kernel/hal/kbd/$(PLATFORM) \
            kernel/fs                  \
            basic

CFLAGS   += -std=gnu99                 \
            -D__$(PLATFORM)__          \
            -DPLATFORM=\"$(PLATFORM)\" \
            -DVERSION=\"$(VERSION)\"

SRC_DIR  := $(addprefix ,$(MODULES))

ASRC     := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.s))
AOBJ     := $(ASRC:.s=.o)
CSRC     := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
COBJ     := $(CSRC:.c=.o)

INCLUDES := $(addprefix -I,$(SRC_DIR))

vpath %.c $(SRC_DIR)
vpath %.s $(SRC_DIR)

OBJ = $(AOBJ) $(COBJ)

linux: $(OBJ)
	$(CC) $(OBJ) $(LFLAGS) -o $(TARGET)

pi: $(OBJ)
	$(LD) $(LFLAGS) -T $(LD_SCR) $(OBJ) -Map $(MAP) -o $(KERNEL)
	$(OBJCOPY) -O binary $(KERNEL) $(IMAGE)

clean:
	rm -f $(TARGET) $(KERNEL) $(IMAGE) $(MAP) $(OBJ)
	find . -name *.o -exec rm -rf {} \;

%.o: %.c
	$(CC) $(INCLUDES) $(CFLAGS) -c -o $*.o $<

%.o: %.s
	$(AS) $(INCLUDES) $(AFLAGS) -o $*.o $<
