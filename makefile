EXE := Typing_stuff

C_VERSION := c99

HEADERS := $(shell find headers -maxdepth 1 -name "*.h")
SRC := $(shell find src -maxdepth 1 -name "*.c")

FLAGS := -I ./headers -Wall -Wextra -g -O2 -std=$(C_VERSION) -fsanitize=address

$(EXE) : $(SRC) $(HEADERS)
	clang -o "$(EXE)" $(SRC)  $(shell pkg-config --cflags --libs raylib) $(FLAGS)

.PHONY : build run clean

build : $(EXE)

clean :
	rm -f $(EXE)

run : $(EXE)
	./$(EXE)