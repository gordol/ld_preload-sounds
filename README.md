To build, run:

    gcc -g -fPIC -shared -Wl,--no-as-needed -ldl -o writeWav.so writeWav.c

Then you can LD_PRELOAD it and capture the output, either into a file or pipe it into aplay:

    LD_PRELOAD=./writeWav.so gcc -g -fPIC -ldl -shared -Wl,--no-as-needed -o writeWav2.so writeWav.c | aplay --file-type raw --rate=44100 --channels=1 --format=S16

Listen here:

* https://soundcloud.com/glowdon/jingy-compiler-1
* https://soundcloud.com/glowdon/the-sounds-of-malloc
* https://soundcloud.com/glowdon/malloc-read-hook-self-compile
