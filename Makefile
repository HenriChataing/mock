SRCDIR=src
OBJDIR=obj
BINDIR=bin

CC=clang++
CFLAGS=
LFLAGS=

EXE=cvm
SRC=$(wildcard $(SRCDIR)/*.cpp)
OBJ=$(SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

main: $(BINDIR)/$(EXE)

$(OBJ): $(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

obj/abstract.o: test/abstract.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(BINDIR)/$(EXE): $(OBJ) obj/abstract.o
	$(CC) $(LFLAGS) $(OBJ) obj/abstract.o -o $@

.PHONY: clean
clean:
	rm -f $(OBJ) $(BINDIR)/$(EXE)
