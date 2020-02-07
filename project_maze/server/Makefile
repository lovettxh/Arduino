CC=g++
LFLAGS=
CFLAGS=-c -Wall -std=c++11 -O2
OBJS=maze.o serialport.o
EXE=maze


$(EXE): $(OBJS)
	$(CC) $(OBJS) -o $(EXE) $(LFLAGS)

maze: maze.o serialport.o
	$(CC) maze.o serialport.o -o maze $(LFLAGS)

maze.o: maze.cpp serialport.h
	$(CC) maze.cpp -o maze.o $(CFLAGS)

serialport.o: serialport.cpp serialport.h
	$(CC) serialport.cpp -o serialport.o $(CFLAGS)

clean:
	@rm -f $(OBJS) $(EXE)
