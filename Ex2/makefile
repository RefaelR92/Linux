CFLAGS = -Wall -g3 -w
LDFLAGS = -lm 
PTHREAD = -pthread
CC = gcc
PROG = Ex2

# -w - disable the warinigs 
# -g3 - for gdb debugging 


# SRC will hold an array of all the .c files
SRCS := $(subst ./,,$(shell find . -name "*.c"))

# OBJS will hold an array of the corresponding .out to the .c files
OBJS := $(patsubst %.c,%.out,$(SRCS))

all: $(OBJS)

	
%.out: %.c  %.h
	$(CC) -o $(PROG) $(CFLAGS) $^ $(LDFLAGS) $(PTHREAD)  -lmta_rand -lmta_crypt -lcrypto 

clean:
	rm -vf *.o ${PROG} *.log
	
test:
	sudo ./Ex2 -n 256 -l 24 -t 10
