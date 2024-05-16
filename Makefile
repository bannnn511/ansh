build-gcc: main.c
	gcc -o ansh main.c parse.c -Wall -Werror
build:
	cd builddir && meson compile
run:
	./builddir/ansh
clean:
	rm ansh
