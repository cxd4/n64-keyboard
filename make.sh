mkdir -p obj

src="."
obj="$src/obj"

FLAGS_ANSI="\
    -Os \
    -std=c89 \
    -ansi \
    -pedantic \
    -Wall \
    -DPLUGIN_API_VERSION=0x0100"

C_FLAGS=$FLAGS_ANSI

echo Compiling C source code...
cc -o $obj/keyboard.s $src/keyboard.c -S $C_FLAGS

echo Assembling compiled sources...
as -o $obj/keyboard.o $obj/keyboard.s --statistics

echo Linking assembled object files...
ld -o $obj/keyboard.so $obj/keyboard.o -lc -s --shared
