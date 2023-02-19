# compilers
# CC=gcc # linux
OPEN2X=arm-openwiz-linux-gnu-gcc # linux - wiz cross compiler

# settings
CFLAGS=-Wall
#-ansi -pedantic -- both broken by Wiz SDL1.2 currently
LDFLAGS=-fuse-ld=mold
LIBS=-lSDL

SRC=wiz_main.c
BIN=game
# output dirs
# LINUX_OUTPUT=./build/linux
OPEN2X_OUTPUT=./build/wiz

all: $(SRC)

%: wiz_main.c
	$(OPEN2X) $(CFLAGS) -B/usr/bin/mold $< -o $(OPEN2X_OUTPUT)/$(BIN).gpe -L/opt/arm-openwiz-linux-gnu/lib $(LIBS)

.PHONY: clean
clean:
	$(RM) $(LINUX_OUTPUT)/$(BIN)
	$(RM) $(OPEN2X_OUTPUT)/$(BIN).gpe
