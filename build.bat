::cl /W4 /Fe:bin\main src\main.c gdi32.lib user32.lib opengl32.lib
gcc -Wall -Wextra -pedantic -o bin\main src\main.c -lgdi32 -lopengl32 -lxaudio2_8 -lole32
