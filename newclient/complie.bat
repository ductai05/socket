@echo off

set COMPILER=g++

echo Compiling...

%COMPILER% -o client.exe client.cpp ^
    -I"DL2\include" ^
    -I"DL2_IMAGE\include" ^
    -I"DL2_TTF\include" ^
    -L"DL2\lib" ^
    -L"DL2_IMAGE\lib" ^
    -L"DL2_TTF\lib" ^
    -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -mwindows

echo Done!

pause