@ECHO OFF
TITLE Windows Driver Kit 7.1.0

    set target=amd64
REM set target=i386

set DDK=C:\WinDDK\7600.16385.1
set MSVC=%DDK%\bin\x86\amd64
set incl=/I"%DDK%\inc\crt" /I"%DDK%\inc\api"
set libs=/LIBPATH:"%DDK%\lib\crt\%target%"

set C_FLAGS=/c /Wall /O1 /Os /Ob1 /Oi /Oy /GS- /MD /TC /Fa
set LINK_FLAGS=%libs% /DLL /NOENTRY /FILEALIGN:512 /MERGE:.rdata=.text

%MSVC%\cl.exe %CD%\config.c %CD%\analog.c %CD%\keyboard.c %CD%\main.c %incl% %C_FLAGS% /link %LINK_FLAGS%
%MSVC%\link.exe %LINK_FLAGS% /SUBSYSTEM:CONSOLE,5.2 config.obj analog.obj keyboard.obj main.obj

pause
