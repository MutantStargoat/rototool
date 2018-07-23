ccsrc = $(wildcard src/*.cc) \
		$(wildcard src/video/*.cc) \
		$(wildcard src/app/*.cc) \
		$(wildcard src/clip/*.cc) \
		$(wildcard src/gmath/*.cc) \
		$(wildcard src/utk/*.cc)
csrc = $(wildcard src/*.c) \
	   $(wildcard src/dtx/*.c) \
	   $(wildcard src/utk/*.c)
obj = $(ccsrc:.cc=.o) $(csrc:.c=.o)
dep = $(obj:.o=.d)
proj = rototool

warn = -pedantic -Wall -Wno-deprecated-declarations
dbg = -g
opt = -O0

def = -DNO_FREETYPE
inc = -Isrc -Isrc/utk

CFLAGS = $(warn) $(dbg) $(opt) $(def) $(inc) `pkg-config --cflags sdl2`
CXXFLAGS = $(warn) $(dbg) $(opt) $(def) $(inc) `pkg-config --cflags sdl2`
LDFLAGS = $(libgl) $(syslibs) `pkg-config --libs sdl2` -lavformat -lavcodec -lavutil

sys ?= $(shell uname -s | sed 's/MINGW.*/mingw/')

ifeq ($(sys), mingw)
	obj = $(src:.cc=.w32.o) $(csrc:.c=.w32.o)
	dep	= $(obj:.o=.d)

	bin = $(proj).exe
	libgl = -lopengl32 -lglu32 -lglew32
	syslibs = -lmingw32 -lwinmm -mwindows
else
	bin = $(proj)
	libgl = -lGL -lGLU -lGLEW
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

.PHONY: font
font:
	font2glyphmap -size 16 -range 33-127 -o data/font.glyphmap \
		/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf
