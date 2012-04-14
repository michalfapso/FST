#CXX = clang++ -v
CXX = g++
CXXFLAGS = -g -Wall -std=c++0x -I../include
LDFLAGS = -lprofiler -lfst -ldl
#LDFLAGS = -lprofiler -ltcmalloc -lfst -ldl 

EXEC=fstcompose_spec_symbol

SOURCES:=$(shell ls *.cpp)
#These are the dependency files, which make will clean up after it creates them
DEPFILES:=$(patsubst %.cpp,.deps/%.P,$(SOURCES))
OBJFILES:=$(patsubst %.cpp,.objs/%.o,$(SOURCES))

.PHONY: clean libmlf all

all : CXXFLAGS += -O0
all : $(EXEC)

dist : CXXFLAGS += -O3
dist : $(EXEC)

$(EXEC) : $(OBJFILES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

.objs/%.o : %.cpp
	@mkdir -p .objs
	@mkdir -p .deps
	$(CXX) -MMD -c -o $@ $< $(CXXFLAGS)
	@cp .objs/$*.d .deps/$*.P; \
		sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
		-e '/^$$/ d' -e 's/$$/ :/' < .objs/$*.d >> .deps/$*.P; \
		rm -f .objs/$*.d

-include $(DEPFILES)

clean :
	rm -r .objs .deps $(EXEC) 


