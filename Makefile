src = $(wildcard src/*.cc)
obj = $(src:.cc=.o)
bin = rototool

CXXFLAGS = -pedantic -Wall -g

$(bin): $(obj)
	$(CXX) -o $@ $(obj) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)
