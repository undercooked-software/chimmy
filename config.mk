# paths
LINUX_DIR=./build/linux
OPEN2X_DIR=./build/wiz

# binaries
BIN=game
OPEN2X_BIN=${BIN}.gpe

# utilities
OPEN2X_SDL=/opt/arm-openwiz-linux-gnu/bin/sdl-config

# includes and libs
EXT=-Iext

# flags
#CFLAGS=-g -ansi -pedantic -Wall
CFLAGS=-ansi -pedantic -Wall -Wno-unused-function -Os

# compilers and linkers
CC=cc
OPEN2X=arm-openwiz-linux-gnu-gcc
MOLD=-fuse-ld=mold
