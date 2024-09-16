g++ -m64 -Os -Ofast -Wall -Wextra -Wpedantic -std=c++2b -o test src/*.cpp -lGL -lglfw -lm -lpthread
x86_64-w64-mingw32-g++ -m64 -Os -Ofast -Wall -Wextra -Wpedantic -std=c++2a -o test src/*.cpp -I vendor/include -L vendor/lib -lglfw3 -lopengl32 -lgdi32 -lkernel32 -luser32 -lm -lpthread
