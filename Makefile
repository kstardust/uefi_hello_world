PLATFORM?=x86_64
ifeq ($(PLATFORM), aarch64)
    EFI_FILE=BOOTAA64.EFI
else
    PLATFORM=x86_64
    EFI_FILE=BOOTX64.EFI
endif

CFLAGS=-target $(PLATFORM)-unknown-windows \
       -ffreestanding \
       -fshort-wchar \
       -mno-red-zone \
       -Ignu-efi/inc -Ignu-efi/inc/$(PLATFORM) -Ignu-efi/inc/protocol
LDFLAGS=-target $(PLATFORM)-unknown-windows \
        -nostdlib \
        -Wl,-entry:efi_main \
        -Wl,-subsystem:efi_application \
        -fuse-ld=lld-link
IMG_NAME=ultimate_hello_$(PLATFORM).img
BIN_NAME=ultimate_hello_$(PLATFORM).bin

all: ${BIN_NAME}

hello.o: hello.c
	clang $(CFLAGS) -c -o $@ $^

data.o: gnu-efi-data.c
	clang $(CFLAGS) -c -o $@ $^

$(EFI_FILE): hello.o data.o
	clang $(LDFLAGS) -o $@ $^

${IMG_NAME}: $(EFI_FILE)
	dd if=/dev/zero of=$@ bs=1k count=1440
	mformat -i $@ -f 1440 ::
	mmd -i $@ ::/EFI
	mmd -i $@ ::/EFI/BOOT
	mcopy -i $@ $(EFI_FILE) ::/EFI/BOOT

${BIN_NAME}: ${IMG_NAME}
	mkgpt -o $@ --image-size 4096 --part $^ --type system

clean:
	rm -f hello.o data.o $(EFI_FILE) ${IMG_NAME} ${BIN_NAME}

.PHONY: all clean

