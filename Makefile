CXX := g++
CXXFLAGS := -std=c++17 -DGLEW_STATIC -I ./ -fPIC
LIBS := -lGLEW -lGL -lglfw
LDFLAGS :=
DLLFLAGS :=
ifeq ($(BUILD),RELEASE)
OPT := -O3
else
OPT := -g -DSCROLLS_DEBUG
endif


sources := $(wildcard */*.cc)
headers := $(wildcard */*.h)
allobjects := $(patsubst %,obj/%, $(patsubst %.cc,%.o, $(sources)))

objects = $(foreach dir,$(1),$(patsubst %.cc,obj/%.o,$(wildcard $(dir)/*.cc)))

scrolls: $(call objects,base glgraphics)
	$(CXX) $(CXXFLAGS) $(OPT) $^ -o scrolls $(LDFLAGS) $(LIBS)

$(allobjects): obj/%.o : %.cc $(headers)
	$(CXX) $(CXXFLAGS) $(OPT) -c $< -o $@
