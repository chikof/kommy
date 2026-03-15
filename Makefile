CC     = gcc
SDK_SRC = wooting-rgb-sdk/src
HIDAPI_CFLAGS = $(shell pkg-config --cflags hidapi-hidraw)
HIDAPI_LIBS   = $(shell pkg-config --libs hidapi-hidraw)

CFLAGS  = -Wall -Wextra -O2 -I$(SDK_SRC) $(HIDAPI_CFLAGS)
LDFLAGS = $(HIDAPI_LIBS) -lpthread

SDK_OBJS = $(SDK_SRC)/wooting-rgb-sdk.o $(SDK_SRC)/wooting-usb.o
SRC = src/main.c
BIN = kommy

.PHONY: all clean

all: $(BIN)

$(SDK_SRC)/%.o: $(SDK_SRC)/%.c
	$(CC) -O2 -fPIC -I$(SDK_SRC) $(HIDAPI_CFLAGS) -c $< -o $@

$(BIN): $(SRC) $(SDK_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(BIN) $(SDK_OBJS)
