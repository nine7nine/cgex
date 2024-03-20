CC = gcc
CFLAGS = -Wall -Wextra
LIBCGEX_SRC = libcgex.c
LIBCGEX_HEADER = libcgex.h

TARGETS = cgexd cgexd-cli cgex-gtk
SRC = cgexd.c cgexd-cli.c cgex-gtk.c
OBJ = $(SRC:.c=.o)

ifdef SANITIZE
	SANITIZE_FLAGS = -g -O1 -fsanitize=address
else
	SANITIZE_FLAGS =
endif

all: $(TARGETS)

cgexd: cgexd.c $(LIBCGEX_SRC) $(LIBCGEX_HEADER)
	$(CC) $(CFLAGS) $(SANITIZE_FLAGS) -o $@ $^

cgexd-cli: cgexd-cli.c $(LIBCGEX_SRC) $(LIBCGEX_HEADER)
	$(CC) $(CFLAGS) $(SANITIZE_FLAGS) -o $@ $^

cgex-gtk: cgex-gtk.c $(LIBCGEX_SRC) $(LIBCGEX_HEADER)
	$(CC) $(CFLAGS) $(SANITIZE_FLAGS) `pkg-config --cflags --libs gtk+-3.0` -o $@ $^

.PHONY: clean
clean:
	rm -f $(TARGETS) $(OBJ)

