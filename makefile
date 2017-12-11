CXX=g++ -std=c++11
OBJS=sprite.o window.o main.o world.o random.o ghostAI.o
LFLAGS=-lSDL2 -lSDL2_image
DEPS=Lab309_ADT_Matrix.h Lab309_ADT_MatrixImplementation.h

%.o: %.cpp $(DEPS)
	$(CXX) -c $< $(LFLAGS)
	
main: $(OBJS)
	$(CXX) -o main $(OBJS) $(LFLAGS)
	
clean:
	rm -f *.o
	rm -f main
