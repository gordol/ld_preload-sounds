CC = gcc
CPPFLAGS += -pipe
CFLAGS += -std=gnu99
LDFLAGS += -fPIC -shared -Wl,--no-as-needed
LIBS = -ldl


all: writeWav.so

writeWav.so: writeWav.c support.h support.c
	${CC} ${CPPFLAGS} ${CFLAGS} ${LDFLAGS} ${LIBS} -o $@ $(filter-out %.h,$^)

clean:
	rm -f writeWav.so writeWav.o support.o

DESTDIR=/
prefix=/usr/local


install: writeWav.so
	mkdir -p ${DESTDIR}/${prefix}/lib
	mkdir -p ${DESTDIR}/${prefix}/bin
	install -m 644 writeWav.so ${DESTDIR}/${prefix}/lib/
	install -m 755 writeWav ${DESTDIR}/${prefix}/bin/
