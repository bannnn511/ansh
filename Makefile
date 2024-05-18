build-gcc: main.c
	gcc -o ansh main.c parse.c print.c -Wall -Werror
init:
	meson setup builddir
build:
	meson compile -C builddir
debug:
	./builddir/ansh -d
run:
	./builddir/ansh
clean:
	rm ansh
	rm -rf builddir
