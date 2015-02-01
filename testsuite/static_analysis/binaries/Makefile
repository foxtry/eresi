ELFSH=../../../elfsh/elfsh32

all: all32
all32: test
test:	bin
	@ echo ===================================
	@ echo ========== OBJDUMP ================
	@objdump -R bin | tee tRel.objdump.log
	@ echo ===================================
	@ echo ========== ELFSH ==================
	@${ELFSH} -f bin -rel | tee tRel.elfsh.log

bin:
	cc -m32 -c bin.c
	cc -m32 -fPIC -shared -o libfunc.so libfunc.c
	cc -m32 -o bin bin.o -L. -lfunc

fclean:
clean:
	rm -f libfunc.so bin.o bin *~
