# compilers
CC=gcc # linux x86_64
OPEN2X=arm-openwiz-linux-gnu-gcc # linux - wiz cross compiler

# settings
CFLAGS=-Wall -ansi -pedantic -O2
LDFLAGS=-fuse-ld=mold
LIBS=-lSDL

SRC=wiz_main.c
BIN=game
# output dirs
LINUX_OUTPUT=./build/linux
OPEN2X_OUTPUT=./build/wiz

all: $(SRC)

%: wiz_main.c
	$(OPEN2X) $(CFLAGS) -I./SDL-1.2.13/include -B/usr/bin/mold $< -o $(OPEN2X_OUTPUT)/$(BIN).gpe -L/opt/arm-openwiz-linux-gnu/lib -L./lib/wiz $(LIBS)

.PHONY: clean
clean:
	$(RM) $(LINUX_OUTPUT)/$(BIN)
	$(RM) $(OPEN2X_OUTPUT)/$(BIN).gpe
