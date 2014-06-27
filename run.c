void main(int argc, char* argv[]) {
  system("gcc -o build/icsh lib.c src/bind/bind_lib.c src/bind/glue.c main.c -lncurses");
  system("./build/icsh");
//gcc -g -o build/debug lib.c src/bind/bind.c src/bind/glue.c main.c -lncurses && gdb build/debug
}
