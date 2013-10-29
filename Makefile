CC=gcc
SOURCES=main.c
OBJECTS=$(SOURCES:%.c=%.o)
EXECUTABLE=task2


$(EXECUTABLE): $(OBJECTS)
	$(CC) -o $(EXECUTABLE) $(OBJECTS)


$(OBJECTS): $(SOURCES)
	$(CC) -c $?


clean:
	rm -rf *.o $(EXECUTABLE)
