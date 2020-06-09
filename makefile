pong: pong.o physics.o
	g++ -o pong -lSDL -lSDL_image pong.o physics.o

pong.o: pong.cpp
	g++ `sdl-config --cflags` -c pong.cpp

physics.o: physics.cpp physics.h
	g++ -c physics.cpp

