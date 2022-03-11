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


SOURCES := $(wildcard */*.cc)
HEADERS := $(wildcard */*.h)
OBJECTS := $(patsubst %,obj/%, $(patsubst %.cc,%.o, $(SOURCES)))

scrolls: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OPT) $(OBJECTS) -o scrolls $(LDFLAGS) $(LIBS)

$(OBJECTS): obj/%.o : %.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) $(OPT) -c $< -o $@
