CC = gcc
LD = ld
AS = as

CFLAGS = -m32 -g -ffreestanding -nostdlib \
					-fno-pie -fno-pic \
         -fno-stack-protector -Wall -Wextra
CFLAGS += -DAVL_SMP=2
ASFLAGS = --32 -g
LDFLAGS = -m elf_i386 -T linker.ld

QEMU = qemu-system-i386 -m 512M -smp 2 -cdrom
QEMU_FLAGS = -enable-kvm -cpu host -serial mon:stdio -d int,cpu_reset -D qemu.log -no-reboot -no-shutdown

OBJ = \
	obj/boot.o \
	obj/ap_tramp.o \
	obj/multiboot.o \
	obj/gdt.o \
	obj/idt.o \
	obj/exception.o \
	obj/smp_bring.o \
	obj/spinlock.o \
	obj/cpu.o \
	obj/smp.o \
	obj/lapic.o \
	obj/isr.o \
	obj/uart.o \
	obj/vga.o \
	obj/kernel.o \
	obj/kstring.o \
	obj/pmm.o \
	obj/paging.o \
	obj/pic.o \
	obj/syscalls.o \
	obj/irq.o \
	obj/keyboard.o \
	obj/timer.o \
	obj/io.o \
	obj/usermode_c.o \
	obj/usermode_s.o \
	obj/test_user.o  \
	obj/ap_main.o
kernel.elf: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

obj/boot.o: boot/boot.S
obj/multiboot.o: boot/multiboot.c
obj/gdt.o: cpu/gdt.c
obj/ap_tramp.o: cpu/ap_tramp.S
obj/idt.o: cpu/idt.c
obj/lapic.o: cpu/lapic.c
obj/smp.o: cpu/smp.c
obj/cpu.o: cpu/cpu.c
obj/smp_bring.o: cpu/smp_bring.c
obj/exception.o: cpu/exception.c
obj/isr.o: cpu/isr.s
obj/spinlock.o: cpu/spinlock.c
obj/uart.o: drivers/uart.c
obj/vga.o: drivers/vga.c
obj/kernel.o: kernel/kernel.c
obj/kstring.o: kernel/kstring.c
obj/ap_main.o: kernel/ap_main.c
obj/pmm.o: mm/pmm.c
obj/paging.o: mm/paging.c
obj/pic.o: cpu/pic.c
obj/syscalls.o: interrupt/syscalls.c
obj/irq.o: interrupt/irq.c
obj/timer.o: drivers/timer.c
obj/keyboard.o: drivers/keyboard.c
obj/io.o: io/io.c
obj/usermode_c.o: user/usermode.c
obj/usermode_s.o: user/usermode.s
obj/test_user.o: user/test_user.s
obj/%.o:
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

ISO_DIR = iso
ISO = kernel.iso

kernel.iso: kernel.elf
	mkdir -p $(ISO_DIR)/boot/grub
	cp kernel.elf $(ISO_DIR)/boot/
	echo 'set timeout=0' > $(ISO_DIR)/boot/grub/grub.cfg
	echo 'set default=0' >> ${ISO_DIR}/boot/grub/grub.cfg
	echo 'menuentry "kernel" {' >> $(ISO_DIR)/boot/grub/grub.cfg
	echo '    multiboot2 /boot/kernel.elf' >> $(ISO_DIR)/boot/grub/grub.cfg
	echo '    boot' >> $(ISO_DIR)/boot/grub/grub.cfg
	echo '}' >> $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $@ $(ISO_DIR)

run: kernel.iso
	$(QEMU) $< $(QEMU_FLAGS)
run-gdb: kernel.iso
	$(QEMU) $< $(QEMU_FLAGS) -display none -s -S

clean:
	rm -rf obj *.elf *.iso iso qemu.log
