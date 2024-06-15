ifdef PS5_PAYLOAD_SDK
    include $(PS5_PAYLOAD_SDK)/toolchain/prospero.mk
else
    $(error PS5_PAYLOAD_SDK is undefined)
endif

CFLAGS := -O3 -I./TestNes -I./TestNes/MMC \
          $(shell ${PS5_SYSROOT}/bin/sdl2-config --cflags --libs) \
          -lkernel_sys -lSDL2main -lSDL2_mixer -lSDL2

SRCS := TestNes/capu.c TestNes/ccpu.c TestNes/main.c  TestNes/cnes.c TestNes/cppu.c TestNes/cwnd.c \
        TestNes/MMC/mapper_aorom.c TestNes/MMC/mapper.c \
        TestNes/MMC/mapper_cnrom.c TestNes/MMC/mapper_mmc1.c \
        TestNes/MMC/mapper_mmc2.c TestNes/MMC/mapper_mmc3.c \
        TestNes/MMC/mapper_nrom.c TestNes/MMC/mapper_unrom.c

cnes.elf: $(SRCS)
	  $(CC) $(CFLAGS) -o $@ $(SRCS)

clean:
	  rm -f cnes.elf
