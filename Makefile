OBJ = colorful.o
BIN = colorful

all: ${BIN}

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

colorful.o: colorful.c colorful.h
	${CC} -c -o $@ $< -I${X11INC}

${BIN}: ${OBJ}
	${CC} -o $@ ${OBJ} -L${X11LIB} -lX11

run: ${BIN}
	./${BIN}

clean:
	rm -f ${BIN} ${OBJ}

.PHONY: all clean
