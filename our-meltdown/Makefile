CC=g++
CFLAGS=-I. --std=c++11 -O0 --no-pie -fPIC
CC_VER_GTE48 := $(shell expr `$(CC) -dumpversion | cut -f1-2 -d.` \>= 4.8)

OBJ1=attacker.o
OBJ2=victim.o

ifeq ($(CC_VER_GTE48), 1)
CFLAGS += -mrtm -DHAS_COMPILER_RTM_SUPPORT
endif

%.o: %.cc
	$(CC) $(CFLAGS) -c -o $@ $< 

all: attacker victim

attacker: $(OBJ1)
	$(CC) $(CFLAGS) -o $@ $^

victim: $(OBJ2)
	$(CC) $(CFLAGS) -o $@ $^


.PHONY: clean

clean:
	rm -f attacker *.o *~ *.out
	rm -f victim *.o *~ *.out
