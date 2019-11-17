CROSS_COMPILE ?= arm-none-eabi-

all: startup.s
	@$(CROSS_COMPILE)as *.s -o startup.o
	@$(CROSS_COMPILE)ld -o startup -T memmap startup.o
	@$(CROSS_COMPILE)objcopy startup startup.bin -O binary
	@$(CROSS_COMPILE)objdump -DSx -b binary -marm startup.bin > startup.lst
	@./gerador_thumb < startup.lst > thumb.txt
	@g++ -std='c++14' main.cpp -o main
	@./main < thumb.txt
	

clean:
	rm *.o *.bin *.lst
