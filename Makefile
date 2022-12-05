OBJ = colorful.o clients.o xinerama.o shortcuts.o logger.o
BIN = colorful

all: ${BIN}

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

colorful.o: colorful.c colorful.h logger.h clients.h xinerama.h shortcuts.h colorful_shortcuts.h
	${CC} -c -o $@ $< -I${X11INC}

clients.o: clients.c logger.h clients.h colorful.h
	${CC} -c -o $@ $< -I${X11INC}

xinerama.o: xinerama.c logger.h colorful.h clients.h xinerama.h
	${CC} -c -o $@ $< -I${X11INC}

shortcuts.o: shortcuts.c clients.h colorful.h colorful_shortcuts.h shortcuts.h
	${CC} -c -o $@ $< -I${X11INC}

logger.o: logger.c logger.h
	${CC} -c -o $@ $< -I${X11INC}

${BIN}: ${OBJ}
	${CC} -o $@ ${OBJ} -L${X11LIB} -lX11 -lXinerama

run: ${BIN}
	./${BIN}

clean:
	rm -f ${BIN} ${OBJ}

.PHONY: all clean
