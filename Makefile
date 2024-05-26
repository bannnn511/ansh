build-gcc: main.c
	gcc -o ansh main.c parse.c utils.c -Wall -Werror -lpthread
wish:
	rm -f wish
	gcc -o wish  main.c parse.c utils.c -Wall -Werror -lpthread
test:
	rm -f wish
	gcc -o wish  main.c parse.c utils.c -Wall -Werror -lpthread
	./test-wish.sh
init:
	meson setup builddir
build:
	meson compile -C builddir
build-run:
	meson compile -C builddir
	./builddir/ansh
debug:
	./builddir/ansh -d
run:
	./builddir/ansh
clean:
	rm ansh
	rm -rf builddir
