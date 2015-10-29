mkdir -p obj

src="."
obj="$src/obj"

FLAGS_ANSI="\
    -Os \
    -pedantic \
    -Wall \
    -DPLUGIN_API_VERSION=0x0100"

C_FLAGS=$FLAGS_ANSI

echo Compiling C source code...
cc -o $obj/main.s               $src/main.c     -S $C_FLAGS
cc -o $obj/keyboard.s           $src/keyboard.c -S $C_FLAGS

echo Assembling compiled sources...
as -o $obj/main.o               $obj/main.s     --statistics
as -o $obj/keyboard.o           $obj/keyboard.s --statistics

echo Linking assembled object files...
gcc -o $obj/keyboard.so $obj/keyboard.o $obj/main.o -s --shared
