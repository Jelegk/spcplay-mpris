.SUFFIXES:

project    := mprisserver-spcplay
outdir     := build
files      := main.cpp
CC         := gcc
CXX        := g++
CFLAGS     := -I./external/include -std=c11 -Wall -Wextra -pedantic -Wno-unused-parameter
CXXFLAGS   := -I./external/include -std=c++20 -Wall -Wextra -pedantic -Wno-unused-parameter
LDLIBS     := -lfmt $(shell pkg-config --libs sdbus-c++) -lspctag
flags_deps  = -MMD -MP -MF $(@:.o=.d)


objs := $(patsubst %,$(outdir)/%.o,$(files))

.PHONY: all clean install uninstall

all: $(outdir)/$(project)

install:

uninstall:

clean:
	rm -rf $(outdir)

$(outdir)/$(project): $(outdir) $(objs)
	$(CXX) $(objs) -o $@ $(LDLIBS)

$(outdir):
	mkdir -p $(outdir)

-include $(outdir)/*.d

$(outdir)/%.cpp.o: %.cpp
	$(CXX) $(CXXFLAGS) $(flags_deps) -c $< -o $@

$(outdir)/%.c.o: %.c
	$(CC) $(CFLAGS) $(flags_deps) -c $< -o $@
