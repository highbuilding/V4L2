EXE=parse_bmp
CC=${CROSS_COMPILE}gcc
OBJ=framebuffer_test.o jp_bmp.o jp_convert.o new_8to16_convert.o new_8to24_convert.o new_8to32_convert.o
CFLAGS=-g
${EXE}:${OBJ}
	${CC} $^ -o $@
clean:
	rm ${EXE} ${OBJ} -f