#!/bin/sh

NO_OUTPUT="/dev/null"

{
  pushd "build"
    mkdir "debug" "release"
    #pushd "debug"
    #  mkdir "x86" "x64"
    #popd
    #pushd "release"
    #  mkdir "x86" "x64"
    #popd
  popd
} &> $NO_OUTPUT

PROJECT_ALIAS="chimmy"
PREPROCESSOR="-DSHIP_MODE=0 -DUSE_GAMEPAD"
WARNINGS="-Wall -Wextra"

DEBUG_COMPILER_FLAGS="-g -O0 -disable-llvm-optzns -fno-elide-constructors"
RELEASE_COMPILER_FLAGS="-O3 -foptimization-record-file=$PROJECT_ALIAS.opt"

EXTERNAL_LIBS="-lSDL2"

COMMAND="$1"
if [ "$COMMAND" = "clean" ]; then
  {
    pushd "build"
      rm -rf *
    popd
  } &> $NO_OUTPUT
  exit 0
fi

if [ "$COMMAND" = "debug" ]; then CFLAGS=$DEBUG_COMPILER_FLAGS; fi
if [ "$COMMAND" = "release" ]; then CFLAGS=$RELEASE_COMPILER_FLAGS; fi

# https://stackoverflow.com/a/9097530
# Thought this was a pretty clever way of handling such a situation that is shell independent.
if [ "x$CFLAGS" != "x" ]; then
  BUILD_DIR="build/${COMMAND}"
  clang++ $PREPROCESSOR $CFLAGS $WARNINGS -o \
    "${BUILD_DIR}/${PROJECT_ALIAS}_client" \
    "macos_${PROJECT_ALIAS}.cpp" \
    $EXTERNAL_LIBS
else
  echo "${0} > Please specify either 'debug' or 'release' to build."
fi
