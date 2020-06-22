PREFIX = ~/.local

CC = gcc
LIBS = -lX11 -lpthread -lpulse
FLAGS = -std=c99 -Wall -pedantic -Os -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=2
SRC = battery.c clock.c layout.c network.c volume.c
OBJ = ${SRC:.c=.o}


all: dwms tint2s

%.o: %.c structs.h
	${CC} -c ${FLAGS} -o $@ $<

dwms: dwms.o ${OBJ} dwms.h tuning.h
	${CC} ${LIBS} ${OBJ} $< -o $@

tint2s: tint2s.o ${OBJ} tint2s.h tuning.h
	${CC} ${LIBS} ${OBJ} $< -o $@

install: all
	mkdir -p ${PREFIX}/bin
	cp -f dwms ${PREFIX}/bin
	chmod 755 ${PREFIX}/bin/dwms
	cp -f tint2s ${PREFIX}/bin
	chmod 755 ${PREFIX}/bin/tint2s

uninstall:
	rm -f ${PREFIX}/bin/dwms
	rm -f ${PREFIX}/bin/tint2s

clean:
	rm -f ${OBJ} dwms.o dwms tint2s.o tint2s

.PHONY: clean all install uninstall
