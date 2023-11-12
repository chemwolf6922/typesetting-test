CFLAGS ?= -O3
override CFLAGS+=-std=c99 -pedantic -Wall -Wextra
_CFLAGS=$(CFLAGS)
_CFLAGS += -MMD -MP

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)
STATIC_LIBS=libschrift/libschrift.a
SHARED_LIBS=png m

test: $(OBJS) $(STATIC_LIBS)
	$(CC) $(LDFLAGS) -o $@ $^ $(patsubst %,-l%,$(SHARED_LIBS))

%.o: %.c
	$(CC) $(_CFLAGS) -c -o $@ $<

libschrift/libschrift.a:
	$(MAKE) -C libschrift libschrift.a CFLAGS="$(CFLAGS)"

clean:
	$(MAKE) -C libschrift clean
	rm -f $(OBJS) $(DEPS) test

-include $(DEPS)
