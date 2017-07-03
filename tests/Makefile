# vim: set tabstop=8 shiftwidth=8 noexpandtab:

BUILDDIR := build
PROG     := utest
SRCDIR   := ../src

CXXFLAGS += -g -Wall -std=c++11
CXXFLAGS += -include ${CPPUTEST_INC}/CppUTest/MemoryLeakDetectorNewMacros.h
CFLAGS   += -include ${CPPUTEST_INC}/CppUTest/MemoryLeakDetectorMallocMacros.h
CPPFLAGS += -I${CPPUTEST_INC}
CPPFLAGS += -I${SRCDIR}
LDFLAGS  += -L${CPPUTEST_LIBDIR}
LDLIBS   += -lCppUTest

SRCS     := DCF77TimeCode.c utils.c
TESTSRCS := $(wildcard *.cpp)

OBJS     := $(addsuffix .o,$(basename ${SRCS} ${TESTSRCS}))
OBJS	 := $(addprefix ${BUILDDIR}/,${OBJS})

$(shell mkdir -p ${BUILDDIR} > /dev/null)

.PHONY: all
all	: ${PROG}

${BUILDDIR}/%.o	: ${SRCDIR}/%.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c -o $@ $<

${BUILDDIR}/%.o	: %.cpp
	${CXX} ${CXXFLAGS} ${CPPFLAGS} -c -o $@ $<

${PROG}	: ${OBJS}
	${CXX} ${LDFLAGS} -o $@ $^ ${LDLIBS}

.PHONY: view
view	:
	@echo "SRCS    : ${SRCS}"
	@echo "TESTSRCS: ${TESTSRCS}"
	@echo "OBJS    : ${OBJS}"

.PHONY: clean
clean	:
	rm -rf *.core ${BUILDDIR}/*.o ${PROG}

