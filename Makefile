CC = g++
CFLAGS = -D_DEBUG -I/usr/local/include/glm
CPPFLAGS = -g

INCDIR = /usr/local/include
LIBDIR = /usr/local/lib

.cpp.o:
	$(CC) -I$(INCDIR) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

LIBS = -framework OpenGL -framework GLUT -L$(LIBDIR) -lglew -lSOIL
SRCS = main.cpp LoadShaders.cpp 
OBJS = main.o LoadShaders.o 

viewer: $(OBJS) LoadShaders.h 
	g++ -g -o viewer $(OBJS) $(LIBS)

clean:
	rm -f viewer *.o
