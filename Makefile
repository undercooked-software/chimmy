include config.mk

all: x86_64 wiz

x86_64: main.c
	@mkdir -p ${LINUX_DIR}
	@echo '***********  BUILDING x86_64  ***********'
	${CC} ${CFLAGS} `sdl-config --cflags` ${MOLD} -DTARGET_X86_64 $< -o ${LINUX_DIR}/${BIN} `sdl-config --libs`

wiz: wiz_main.c
	@mkdir -p ${OPEN2X_DIR}
	@echo '*********** BUILDING GP2X-WIZ ***********'
	${OPEN2X} ${CFLAGS} `${OPEN2X_SDL} --cflags` -DTARGET_WIZ $< -o ${OPEN2X_DIR}/${OPEN2X_BIN} `${OPEN2X_SDL} --libs`

clean:
	rm -f ${LINUX_DIR}/${BIN}
	rm -f ${OPEN2X_DIR}/${BIN}.gpe

dist: dist-x86_64 dist-wiz

dist-x86_64:
	cp -r data ${LINUX_DIR}
	cp -r ${LINUX_DIR}/data/image/32/* ${LINUX_DIR}/data/image/
	rm -rf ${LINUX_DIR}/data/image/32
	rm -rf ${LINUX_DIR}/data/image/16

dist-wiz:
	cp _chimmy.ini ${OPEN2X_DIR}
	cp run.gpe ${OPEN2X_DIR}
	cp -r data ${OPEN2X_DIR}
	cp -r lib/wiz/* ${OPEN2X_DIR}
	cp -r ${OPEN2X_DIR}/data/image/16/* ${OPEN2X_DIR}/data/image/
	rm -rf ${OPEN2X_DIR}/data/image/32
	rm -rf ${OPEN2X_DIR}/data/image/16

PHONY: all x86_64 wiz clean dist dist-x86_64 dist-wiz
