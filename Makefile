build-gcc: main.c
	gcc -o ansh main.c parse.c print.c -Wall -Werror -lpthread
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
