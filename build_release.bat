echo 1 ICON "rsc\\sudoku.ico" > sudoku.rc
rc sudoku.rc
cl /O2 /Fe:bin\main sudoku.res src\main.c /link /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup gdi32.lib user32.lib opengl32.lib ole32.lib
