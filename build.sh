if [ "$1" == "win32" ]; then
 i686-w64-mingw32-gcc -std=c++14 -m32 -static-libstdc++ -Isrc/SourceAutoRecord/ -Isrc/SourceAutoRecord/Features/ -Ilibs/ -o sar.dll src/SourceAutoRecord/Main.cpp -shared -Wall
else
 g++ -std=c++14 -m32 -fPIC -static-libstdc++ -Isrc/SourceAutoRecord/ -Isrc/SourceAutoRecord/Features/ -Ilibs/ -o sar.so src/SourceAutoRecord/Main.cpp -shared -Wall
fi