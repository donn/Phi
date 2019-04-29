BISON ?= bison
CC ?= cc
CXX ?= c++

SOURCE_DIR = Sources
BUILD_DIR = Intermediates
GRAMMAR_DIR = Grammar
HEADER_DIR = Headers

REFLEX_DIR = Submodules/RE-flex
REFLEX_UNICODE_SRC_DIR = $(REFLEX_DIR)/unicode
REFLEX_LIB_SRC_DIR = $(REFLEX_DIR)/lib
REFLEX_LIB_HEADER_DIR = $(REFLEX_DIR)/include

REFLEX_LIB_SOURCES = $(shell find $(REFLEX_LIB_SRC_DIR) | grep .cpp)
REFLEX_UNICODE_SOURCES = $(shell find $(REFLEX_UNICODE_SRC_DIR) | grep .cpp)

REFLEX_LIB_OBJECTS = $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp,%.o,$(REFLEX_LIB_SOURCES)))
REFLEX_UNICODE_OBJECTS =  $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp,%.o,$(REFLEX_UNICODE_SOURCES)))

REFLEX_MESS = $(REFLEX_DIR)/src/reflex.cpp $(REFLEX_LIB_OBJECTS) $(REFLEX_UNICODE_OBJECTS)

LEX = $(GRAMMAR_DIR)/phi.l
YACC = $(GRAMMAR_DIR)/phi.yy
LEX_OUT = $(addprefix $(BUILD_DIR)/, $(patsubst %,%.cc,$(LEX)))
LEX_HEADER = $(addprefix $(BUILD_DIR)/, $(patsubst %,%.hh,$(LEX)))
YACC_OUT = $(addprefix $(BUILD_DIR)/, $(patsubst %,%.cc,$(YACC)))

YACC_FLAGS = --verbose
C_FLAGS = -pedantic
CPP_LY_FLAGS = -Wno-deprecated-register -std=c++17 $(CPPFLAGS)
CPP_FLAGS = -Wall -pedantic -std=c++17 $(CPPFLAGS)

SOURCES = 
HEADERS =

CPP_LY_SOURCES = $(YACC_OUT) $(LEX_OUT)
CPP_SOURCES = $(shell find Sources | grep .cpp)
CPP_HEADERS = $(shell find Headers | grep .h) $(BUILD_DIR)/git_version.h  $(LEX_HEADER)

LIBRARY_HEADER_PATHS = $(addprefix -I, $(shell find Submodules -mindepth 1 -maxdepth 1)) $(addprefix -I, $(shell find Submodules -name include))
LIBRARY_SOURCES = 
CPP_LIBRARY_SOURCES =

OBJECTS = $(addprefix $(BUILD_DIR)/, $(patsubst %.c,%.o,$(SOURCES))) $(addprefix $(BUILD_DIR)/, $(patsubst %.c,%.o,$(LIBRARY_SOURCES)))

CPP_LY_OBJECTS = $(patsubst %.cc,%.o,$(CPP_LY_SOURCES))
CPP_OBJECTS = $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp,%.o,$(CPP_SOURCES))) $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp,%.o,$(CPP_LIBRARY_SOURCES)))

LD_FLAGS = $(LDFLAGS) -lllvm -lc++ -lSystem

BINARY = phic

# Products

all: CPP_FLAGS += -g -D_DEBUG -DYYDEBUG=1
all: CPP_LY_FLAGS += -g -DYYDEBUG=1
all: C_FLAGS += -g -D_DEBUG 
all: $(BINARY)

release: CPP_FLAGS += -O3
release: CPP_LY_FLAGS += -O3
release: C_FLAGS += -O3
release: $(BINARY)

$(BUILD_DIR)/git_version.h:
	mkdir -p $(@D)
	echo "#ifndef _AUTO_git_version_h" > $@
	echo "#define _AUTO_git_version_h" >> $@
	echo "namespace Phi {" >> $@
	echo "const char* GIT_TAG = \"$(shell git tag | tail -n 1)\";" >> $@
	echo "const char* GIT_VER_STRING = \"$(shell git describe --always --tags)\";" >> $@
	echo "}" >> $@
	echo "#endif // _AUT0_git_version_h" >> $@

$(YACC_OUT): $(YACC)
	mkdir -p $(@D)
	$(BISON) $(YACC_FLAGS) -o $@ -d $^

$(REFLEX_LIB_OBJECTS): $(BUILD_DIR)/%.o : %.cpp $(REFLEX_LIB_HEADER_DIR)
	mkdir -p $(@D)
	$(CXX) $(CPP_LY_FLAGS) -c -I$(REFLEX_LIB_HEADER_DIR) -o $@ $<

$(REFLEX_UNICODE_OBJECTS): $(BUILD_DIR)/%.o : %.cpp $(REFLEX_LIB_HEADER_DIR)
	mkdir -p $(@D)
	$(CXX) $(CPP_LY_FLAGS) -c -I$(REFLEX_LIB_HEADER_DIR) -o $@ $<

Intermediates/reflex: $(REFLEX_MESS)
	mkdir -p $(@D)
	$(CXX) $(CPP_LY_FLAGS) -I $(REFLEX_LIB_HEADER_DIR) -o $@ $^

$(LEX_OUT): $(LEX) $(YACC_OUT) Intermediates/reflex
	mkdir -p $(@D)
	Intermediates/reflex -o $@ --header-file=$(LEX_HEADER) $<

$(LEX_HEADER): $(LEX_OUT)
	@echo "\033[1;32m>> Lex header generated.\033[0m"

$(OBJECTS): $(BUILD_DIR)/%.o : %.c $(HEADERS)
	mkdir -p $(@D)
	$(CC) $(C_FLAGS) -c -I$(HEADER_DIR) -o $@ $<

$(CPP_LY_OBJECTS): %.o : %.cc $(YACC_OUT) $(LEX_OUT) $(CPP_HEADERS) $(HEADERS)
	mkdir -p $(@D)
	$(CXX) $(CPP_LY_FLAGS) -I$(HEADER_DIR) -I$(BUILD_DIR) -I$(BUILD_DIR)/$(GRAMMAR_DIR) -I /usr/local/include/ $(LIBRARY_HEADER_PATHS) -c -o $@ $<

$(CPP_OBJECTS): $(BUILD_DIR)/%.o : %.cpp $(YACC_OUT) $(LEX_OUT) $(CPP_HEADERS) $(HEADERS)
	mkdir -p $(@D)
	$(CXX) $(CPP_FLAGS) -I$(HEADER_DIR) -I$(BUILD_DIR) -I$(BUILD_DIR)/$(GRAMMAR_DIR) $(LIBRARY_HEADER_PATHS) -c -o $@ $<

$(BINARY): $(OBJECTS) $(CPP_OBJECTS) $(CPP_LY_OBJECTS) $(REFLEX_LIB_OBJECTS) $(REFLEX_UNICODE_OBJECTS)
	mkdir -p $(@D)
	$(CXX) $(LD_FLAGS) -o $@ $^
	@echo "\033[1;32m>> Build complete.\033[0m"

.PHONY: clean

clean:
	echo $(ACTION) > ~/action
	rm -rf $(BUILD_DIR)
	rm -f $(BINARY)
	rm -f Examples/*.v
	rm -f ./*.phi
	rm -f *.exe *.stackdump # MSYS2/Windows
	rm -f *.out *.phi.*sv *.dot ./*.png
