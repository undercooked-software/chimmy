CC=gcc
OPEN2X=arm-openwiz-linux-gnu-gcc

CFLAGS=-Wall -ansi -pedantic

LINUX_OUTPUT=./build/linux
OPEN2X_OUTPUT=./build/wiz
BIN=game

# maybe this isn't the best way to do this, but it works for now.
$(shell mkdir -p ./build $(LINUX_OUTPUT) $(OPEN2X_OUTPUT));

all: x86_64 wiz

x86_64: main.c
	@echo '***********  BUILDING x86_64  ***********'
	$(CC) $(CFLAGS) -O2 `sdl-config --cflags` -fuse-ld=mold -DTARGET=x86_64 $< -o $(LINUX_OUTPUT)/$(BIN) `sdl-config --libs`

wiz: wiz_main.c
	@echo '*********** BUILDING GP2X-WIZ ***********'
	$(OPEN2X) $(CFLAGS) -O2 -I./SDL-1.2.13/include -DTARGET=wiz $< -o $(OPEN2X_OUTPUT)/$(BIN).gpe -L/opt/arm-openwiz-linux-gnu/lib -L./lib/wiz -lSDL

PHONY: clean
clean:
	$(RM) $(LINUX_OUTPUT)/$(BIN)
	$(RM) $(OPEN2X_OUTPUT)/$(BIN).gpe

dist: dist-x86_64 dist-wiz

dist-x86_64:
	cp -r data $(LINUX_OUTPUT)
	cp -r $(LINUX_OUTPUT)/data/image/32/* $(LINUX_OUTPUT)/data/image/
	$(RM) -rf $(LINUX_OUTPUT)/data/image/32
	$(RM) -rf $(LINUX_OUTPUT)/data/image/16

dist-wiz:
	cp _chimmy.ini $(OPEN2X_OUTPUT)
	cp run.gpe $(OPEN2X_OUTPUT)
	cp -r data $(OPEN2X_OUTPUT)
	cp -r lib/wiz/* $(OPEN2X_OUTPUT)
	cp -r $(OPEN2X_OUTPUT)/data/image/16/* $(OPEN2X_OUTPUT)/data/image/
	$(RM) -rf $(OPEN2X_OUTPUT)/data/image/32
	$(RM) -rf $(OPEN2X_OUTPUT)/data/image/16

