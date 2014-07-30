all: writeWav.so

writeWav.so: writeWav.c
	${CC} ${CFLAGS} ${LDFLAGS} -fPIC -shared -Wl,--no-as-needed -ldl -o writeWav.so writeWav.c

DESTDIR=/
prefix=/usr/local


install: writeWav.so
	mkdir -p ${DESTDIR}/${prefix}/lib
	mkdir -p ${DESTDIR}/${prefix}/bin
	install -m 644 writeWav.so ${DESTDIR}/${prefix}/lib/
	install -m 755 writeWav ${DESTDIR}/${prefix}/bin/
