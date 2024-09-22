g++ -m64 -Os -Ofast -Wall -Wextra -Wpedantic -std=c++2a -o test src/*.cpp -I vendor/include -L vendor/lib -lglfw3 -lopengl32 -lgdi32 -lkernel32 -luser32 -lm -lpthread
