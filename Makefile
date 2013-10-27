CC=gcc
SOURCES=main.c
OBJECTS=$(SOURCES:%.c=%.o)
EXECUTABLE=task2


$(EXECUTABLE): $(OBJECTS)
	$(CC) -o $(EXECUTABLE) $(OBJECTS)


$(OBJECTS): Sources/main.c
	$(CC) -c $(SOURCES)


clean:
	rm -rf *.o $(EXECUTABLE)
