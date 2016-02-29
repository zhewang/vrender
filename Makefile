CC = g++
CFLAGS = -D_DEBUG
CPPFLAGS = -g

INCDIR = /usr/local/include
LIBDIR = /usr/local/lib

.cpp.o:
	$(CC) -I$(INCDIR) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

LIBS = -framework OpenGL -framework GLUT -L$(LIBDIR) -lglew
SRCS = main.cpp LoadShaders.cpp loadObj.cpp
OBJS = main.o LoadShaders.o loadObj.o

viewer: $(OBJS) LoadShaders.h loadObj.h
	g++ -g -o viewer $(OBJS) $(LIBS)

clean:
	rm -f viewer *.o
