@ECHO OFF
TITLE Windows Driver Kit 7.1.0

REM set target=i386
set target=amd64

set DDK=C:\WinDDK\7600.16385.1
set MSVC=%DDK%\bin\x86\%target%
set incl=/I"%DDK%\inc\crt" /I"%DDK%\inc\api"
set libs=/LIBPATH:"%DDK%\lib\crt\%target%"

set C_FLAGS=/c /Wall /O1 /Os /Ob1 /Oi /Oy /GS- /MD /TC /Fa
set LINK_FLAGS=%libs% /DLL /NOENTRY /FILEALIGN:512 /MERGE:.rdata=.text

%MSVC%\cl.exe %CD%\keyboard.c %incl% %C_FLAGS% /link %LINK_FLAGS%
%MSVC%\link.exe %LINK_FLAGS% /SUBSYSTEM:CONSOLE,5.2 keyboard.obj

pause
