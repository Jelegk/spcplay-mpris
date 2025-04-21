.SUFFIXES:

project      := spcplay-mpris
builddir     := build
incflags := -Isrc
files        := spcplay-mpris.cpp
LDLIBS       := $(shell pkg-config --libs sdbus-c++) # -lfmt (en premier)

CC           := gcc
CXX          := g++
CFLAGS       := -std=c11 -Wall -Wextra -pedantic -Wno-unused-parameter
CXXFLAGS     := -std=c++20 -Wall -Wextra -pedantic -Wno-unused-parameter
flags_deps    = -MMD -MP -MF $(@:.o=.d)


objs := $(patsubst %,$(builddir)/%.o,$(files))

.PHONY: all clean

all: $(builddir)/$(project)

clean:
	rm -rf $(builddir)

$(builddir)/$(project): $(builddir) $(objs)
	$(CXX) $(objs) -o $@ $(LDLIBS)

$(builddir):
	mkdir -p $(builddir)

-include $(builddir)/*.d

$(builddir)/%.cpp.o: src/%.cpp
	$(CXX) $(CXXFLAGS) $(flags_deps) $(incflags) -c $< -o $@

$(builddir)/%.c.o: src/%.c
	$(CC) $(CFLAGS) $(flags_deps) $(incflags) -c $< -o $@
