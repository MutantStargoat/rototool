ccsrc = $(wildcard src/*.cc) \
		$(wildcard src/video/*.cc) \
		$(wildcard src/app/*.cc) \
		$(wildcard src/clip/*.cc) \
		$(wildcard src/gmath/*.cc)
csrc = $(wildcard src/*.c)
obj = $(ccsrc:.cc=.o) $(csrc:.c=.o)
dep = $(obj:.o=.d)
proj = rototool

warn = -pedantic -Wall -Wno-deprecated-declarations
dbg = -g
opt = -O0

inc = -Isrc

CFLAGS = $(warn) $(dbg) $(opt) $(inc)
CXXFLAGS = $(warn) $(dbg) $(opt) $(inc)
LDFLAGS = $(libgl) $(syslibs) -lavformat -lavcodec -lavutil

sys ?= $(shell uname -s | sed 's/MINGW.*/mingw/')

ifeq ($(sys), mingw)
	obj = $(src:.cc=.w32.o) $(csrc:.c=.w32.o)
	dep	= $(obj:.o=.d)

	bin = $(proj).exe
	libgl = -lopengl32 -lglu32 -lfreeglut -lglew32
	syslibs = -lmingw32 -lwinmm -mwindows
else
	bin = $(proj)
	libgl = -lGL -lGLU -lglut -lGLEW
	syslibs = -lm
endif


$(bin): $(obj)
	$(CXX) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

%.d: %.c
	@$(CPP) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

%.d: %.cc
	@$(CPP) $(CXXFLAGS) $< -MM -MT $(@:.d=.o) >$@

%.w32.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

%.w32.o: %.cc
	$(CC) -o $@ $(CXXFLAGS) -c $<


.PHONY: cross
cross:
	$(MAKE) CC=i686-w64-mingw32-gcc CXX=i686-w64-mingw32-g++ sys=mingw

.PHONY: cross-clean
cross-clean:
	$(MAKE) CC=i686-w64-mingw32-gcc CXX=i686-w64-mingw32-g++ sys=mingw clean


.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cleandep
cleandep:
	rm -f $(dep)
