mkdir -p obj

src="."
obj="$src/obj"

FLAGS_ANSI="\
    -Os \
    -ansi \
    -pedantic \
    -Wall \
    -DPLUGIN_API_VERSION=0x0101"

C_FLAGS=$FLAGS_ANSI

echo Compiling C source code...
cc -o $obj/main.s               $src/main.c     -S $C_FLAGS
cc -o $obj/keyboard.s           $src/keyboard.c -S $C_FLAGS
cc -o $obj/analog.s             $src/analog.c   -S $C_FLAGS

echo Assembling compiled sources...
as -o $obj/main.o               $obj/main.s
as -o $obj/keyboard.o           $obj/keyboard.s
as -o $obj/analog.o             $obj/analog.s

echo Linking assembled object files...
gcc -o $obj/keyboard.so $obj/analog.o $obj/keyboard.o $obj/main.o -s --shared
