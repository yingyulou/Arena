all:
	nasm Mbr.s
	gcc -c -m32 -fno-builtin -o Kernel.o Kernel.c
	nasm -f elf Int.s
	nasm -f elf HD.s
	nasm -f elf Start.s
	gcc -c -m32 -fno-builtin -o Test.o Test.c
	ld -m elf_i386 -Ttext-segment 0xc0000000 -e main -o Kernel Kernel.o Int.o HD.o
	ld -m elf_i386 -Ttext-segment 0 -o Test Test.o Start.o
	if [ ! -f Arena.img ]; then dd if=/dev/zero of=Arena.img count=20480; fi
	dd if=Mbr of=Arena.img seek=0 count=1 conv=notrunc
	dd if=Kernel of=Arena.img seek=1 count=99 conv=notrunc
	dd if=Test of=Arena.img seek=1000 count=20 conv=notrunc

clean:
	rm -f *.o
	rm -f Mbr
	rm -f Kernel
	rm -f Test
	rm -f Arena.img*
