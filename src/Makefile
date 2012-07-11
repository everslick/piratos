-include ../Makefile.config

TARGET    = ../bin/$(PROGRAM)

KERNEL    = ../bin/$(PROGRAM).elf
IMAGE     = ../bin/$(PROGRAM).img
MAP       = ../bin/$(PROGRAM).map
LD_SCR    = kernel/boot/pi/linker.ld

CC        = gcc
CPP       = g++

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

CFLAGS   += -D__$(PLATFORM)__            \
            -DPLATFORM=\"$(PLATFORM)\"   \
            -DVERSION=\"$(VERSION)\"

MODULES  := .                            \
            init                         \
            kernel/os                    \
            kernel/os/$(PLATFORM)        \
            kernel/boot                  \
            kernel/boot/$(PLATFORM)      \
            kernel/hal/sd                \
            kernel/hal/sd/$(PLATFORM)    \
            kernel/hal/fb                \
            kernel/hal/fb/$(PLATFORM)    \
            kernel/hal/kbd               \
            kernel/hal/kbd/$(PLATFORM)   \
            kernel/hal/mouse             \
            kernel/hal/mouse/$(PLATFORM) \
            lib/gfx                      \
            term                         \
            cli                          \
            fs                           \
            fs/fat

CPPFLAGS := $(CFLAGS)

CFLAGS   += -std=gnu99

SRC_DIR  := $(addprefix ,$(MODULES))

CPPSRC   := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.cpp))
CPPOBJ   := $(CPPSRC:.cpp=.o)
ASRC     := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.s))
AOBJ     := $(ASRC:.s=.o)
CSRC     := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
COBJ     := $(CSRC:.c=.o)

INCLUDES := $(addprefix -I,$(SRC_DIR))

#vpath %.c $(SRC_DIR)
#vpath %.s $(SRC_DIR)

OBJ = $(AOBJ) $(COBJ) $(CPPOBJ)

linux: $(OBJ)
	$(CPP) $(OBJ) $(LFLAGS) -o $(TARGET)

pi: $(OBJ)
	$(LD) $(LFLAGS) -T $(LD_SCR) $(OBJ) -Map $(MAP) -o $(KERNEL)
	$(OBJCOPY) -O binary $(KERNEL) $(IMAGE)

clean:
	rm -f $(TARGET) $(KERNEL) $(IMAGE) $(MAP) $(OBJ)
	find . -name *.o -exec rm -rf {} \;

%.o: %.cpp
	$(CPP) $(INCLUDES) $(CPPFLAGS) -c -o $*.o $<

%.o: %.c
	$(CC) $(INCLUDES) $(CFLAGS) -c -o $*.o $<

%.o: %.s
	$(AS) $(INCLUDES) $(AFLAGS) -o $*.o $<
