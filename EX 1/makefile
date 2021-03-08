CC = gcc
LIBS = -lutils
DIR = $(shell pwd)

SRCS := $(subst ./,,$(shell find . -name "*.c"))

OBJS := $(patsubst %.c,%.out,$(filter-out utils.c,$(SRCS)))

all: libutils.so $(OBJS)  


libutils.so: utils.c
	$(CC) -shared -fPIC $^ -o $@ 

%.out: %.c
	$(CC) $^ $(LIBS) -Wl,-rpath,$(DIR),--enable-new-dtags -L$(DIR) -o $@ 

clean:
	find . -name "*.out" -exec rm -rf {} \;
	rm -rf *.so
