all:
	gcc -o obj/dre2.o -fPIC -c src/dre2_combined.c -O3
	gcc -shared -o obj/libdre2.so obj/dre2.o -O3

install:
	cp obj/*.so /usr/local/lib
	cp src/*.h /usr/local/include

clean:
	rm -f obj/*
	rm -f dre2 dre2_submatch

test:
	gcc -o dre2 examples/dre2.c -ldre2 -O3
	gcc -o dre2_submatch examples/submatch.c -ldre2 -O3
