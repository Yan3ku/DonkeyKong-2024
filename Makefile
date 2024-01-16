DEBUG    =
CPPFLAGS = $(shell sdl2-config --cflags)
CFLAGS   = -std=c99 -pedantic -Wextra -Wall ${CPPFLAGS} ${DEBUG}
LDFLAGS  = $(shell sdl2-config --libs) ${DEBUG} -lm -lSDL2_image

BIN = prog
SRC = prog.c
OBJ = ${SRC:.c=.o}

all: options ${BIN}

options:
	@echo ${BIN} build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	${CC} -c ${CFLAGS} $<

${BIN}: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm -f ${BIN} ${OBJ}

.PHONY: all options clean
