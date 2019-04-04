.PHONY: all clean

CC=gcc
CFLAGS+=
LDFLAGS+=-lm     # link to math library

TARGET=train test

all: $(TARGET)

train: ./c_cpp/train.c ./c_cpp/hmm.h ./c_cpp/train.h
	$(CC) -o train ./c_cpp/train.c

test: ./c_cpp/test.c ./c_cpp/hmm.h ./c_cpp/test.h
	$(CC) -o test ./c_cpp/test.c

clean:
	$(RM) $(TARGET)   # type make clean to remove the compiled file
