# paths
LINUX_DIR=./build/linux
OPEN2X_DIR=./build/wiz

# binaries
BIN=game
OPEN2X_BIN=${BIN}.gpe

# includes and libs

# flags
#CFLAGS=-g -ansi -pedantic -Wall
CFLAGS=-ansi -pedantic -Wall -Wno-deprecated-declarations -Os

# compilers and linkers
CC=cc
OPEN2X=arm-openwiz-linux-gnu-gcc
MOLD=-fuse-ld=mold
