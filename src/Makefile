MKPATH=../mk/
include $(MKPATH)buildsys.mk

# import source list
include Makefile.src
INCLUDE = $(HDRS)
OBJECTS = $(ANGFILES) $(ZFILES)
SRCS    = ${OBJECTS:.o=.c} ${MAINFILES:.o=.c}
PROG    = poschengband$(PROG_SUFFIX)
# VERSION := $(shell ../scripts/version.sh)
CFLAGS += -DBUILD_ID=${VERSION} -I. -std=c99 -Wdeclaration-after-statement -O0

CLEAN = poschengband.o $(OBJECTS) win/poschengband.res
DISTCLEAN = autoconf.h

export CFLAGS LDFLAGS LIBS

$(PROG): poschengband.o $(MAINFILES)
	$(CC) -o $@ poschengband.o $(MAINFILES) $(LDFLAGS) $(LDADD) $(LIBS)
	@printf "%10s %-20s\n" LINK $@

win/poschengband.res: win/poschengband.rc
	$(RC) $< -O coff -o $@

poschengband.o: $(OBJECTS)
	$(LD) -nostdlib -Wl,-r -o $@ $(OBJECTS)
	@printf "%10s %-20s\n" LINK $@

splint:
	splint -f .splintrc ${OBJECTS:.o=.c} main.c main-gcu.c

