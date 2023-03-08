CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Wshadow
DEBUG_FLAGS   = -O0 -DDEBUG -g
RELEASE_FLAGS = -O3 -DNDEBUG
OBJ = main.o
TARGET_PROGRAM = attempt-vulk-c

all: CFLAGS += $(RELEASE_FLAGS)
all: $(TARGET_PROGRAM)

debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(TARGET_PROGRAM)

$(TARGET_PROGRAM): $(OBJ)
	$(CC) -o $(TARGET_PROGRAM) $(OBJ) -lm -lglfw -lvulkan -lassimp

main.o: main.c
	$(CC) $(CFLAGS) -I./include -c main.c -o main.o

clean:
	@echo Cleaning up...
	@rm -f *.o
	@rm -f $(TARGET_PROGRAM)
	@echo Done.
