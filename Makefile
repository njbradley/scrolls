CXX := g++
CXXFLAGS := -std=c++17 -DGLEW_STATIC -I ./ -fPIC
LDFLAGS :=
DLLFLAGS :=

ifeq ($(OS),Windows_NT)
LIBS := -lmingw32 -luser32 -lgdi32 -lshell32 -lglew32 -lglfw3 -lopengl32

else
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
LIBS := -lGLEW -lGL -lglfw

endif
ifeq ($(UNAME_S),Darwin)
LIBS := -L /opt/homebrew/lib -lGLEW -lglfw -framework CoreVideo -framework OpenGL -framework IOKit
CXXFLAGS := -std=c++17 -DGLEW_STATIC -I ./ -fPIC -I /opt/homebrew/include

endif
endif

ifeq ($(BUILD),RELEASE)
OPT := -O3
else
ifeq ($(BUILD),PROFILE)
OPT := -O3 -g
else
OPT := -g -DSCROLLS_DEBUG
endif
endif


sources := $(wildcard */*.cc)
headers := $(wildcard */*.h)
allobjects := $(patsubst %,obj/%, $(patsubst %.cc,%.o, $(sources)))

objects = $(foreach dir,$(1),$(patsubst %.cc,obj/%.o,$(wildcard $(dir)/*.cc)))

scrolls: $(call objects,base glgraphics)
	$(CXX) $(CXXFLAGS) $(OPT) $^ -o scrolls $(LDFLAGS) $(LIBS)

$(allobjects): obj/%.o : %.cc $(headers)
	$(CXX) $(CXXFLAGS) $(OPT) -c $< -o $@
