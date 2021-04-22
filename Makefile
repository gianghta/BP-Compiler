CC = clang
LD = clang++
CFLAGS = -g `llvm-config --cflags` -Wall -I$(INCLUDE)
LDFLAGS = `llvm-config --cxxflags --ldflags --libs all --system-libs`
SRC = src
OBJ = obj
INCLUDE = src/include
SRCS = $(wildcard $(SRC)/*.c)
OBJS = $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))

BINDIR = bin
BIN = $(BINDIR)/bp.out

all:$(BIN)

$(BIN): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -r $(BINDIR)/*.out $(OBJ)/*.o ./*.bc