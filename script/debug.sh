gcc -g -o build/debug lib.c src/bind/bind.c src/bind/glue.c main.c -lncurses && gdb build/debug
