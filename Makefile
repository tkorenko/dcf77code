
BUILDDIR:= build
SRCDIR	:= src
CFLAGS	+= -g -Wall -pedantic
CFLAGS	+= -std=c99 -D_GNU_SOURCE

$(shell mkdir -p ${BUILDDIR} > /dev/null)

SRCS	:= $(notdir $(wildcard ${SRCDIR}/*.c))
OBJS	:= $(addprefix ${BUILDDIR}/,$(addsuffix .o,$(basename ${SRCS})))
PROG	:= dcfcode

all	: ${PROG}

${BUILDDIR}/%.o	: ${SRCDIR}/%.c
	${CC} ${CFLAGS} -c -o $@ $<

${PROG}	: ${OBJS}
	${CC} ${LDFLAGS} -o $@ $^ ${LDLIBS}

.PHONY	: view
view	:
	@echo "SRCS: ${SRCS}"
	@echo "OBJS: ${OBJS}"
	@echo "PROG: ${PROG}"

.PHONY	: clean
clean	:
	rm -rf ${PROG} *.core ${BUILDDIR}

