mkdir -p obj

src="."
obj="$src/obj"

FLAGS_ANSI="\
    -Os \
    -ansi \
    -pedantic \
    -Wall \
    -fPIC \
    -DPLUGIN_API_VERSION=0x0101"

C_FLAGS=$FLAGS_ANSI

echo Compiling C source code...
cc -o $obj/main.s               $src/main.c     -S $C_FLAGS
cc -o $obj/keyboard.s           $src/keyboard.c -S $C_FLAGS
cc -o $obj/analog.s             $src/analog.c   -S $C_FLAGS
cc -o $obj/config.s             $src/config.c   -S $C_FLAGS

echo Assembling compiled sources...
as -o $obj/main.o               $obj/main.s
as -o $obj/keyboard.o           $obj/keyboard.s
as -o $obj/analog.o             $obj/analog.s
as -o $obj/config.o             $obj/config.s

echo Linking assembled object files...
gcc -o $obj/keyboard.so $obj/config.o $obj/analog.o $obj/keyboard.o $obj/main.o -s --shared
