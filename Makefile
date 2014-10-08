CC = clang
LD = $(CC)

CFLAGS = -c -std=c99 -Wall -O2 -I. -Icctools/include -DLTO_SUPPORT=1
LDFLAGS = -L.

AR = ar -cvr

LIB = libPrivSymPoke.a
LIBS = -lPrivSymPoke
TOOL = dumpsym

LIB_OBJECTS = $(patsubst cctools/libstuff/%.c, obj/%.o, $(wildcard cctools/libstuff/*.c))
LIB_OBJECTS += obj/nm.o obj/PrivSymPoke.o
TOOL_OBJECTS = obj/dumpsym.o

all: $(LIB) $(TOOL)

$(TOOL): $(LIB) $(TOOL_OBJECTS)
	$(LD) $(LDFLAGS) $(LIBS) -o $@ $^

$(LIB): $(LIB_OBJECTS)
	$(AR) $@ $^

obj/%.o: cctools/libstuff/%.c
	$(CC) $(CFLAGS) -o $@ $<

obj/%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm $(LIB_OBJECTS) $(TOOL_OBJECTS) $(LIB) $(TOOL)

.PHONY: all clean
