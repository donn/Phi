# Detect Tools
BISON ?= bison
CC ?= cc
CXX ?= c++
LLVM_CONFIG ?= llvm-config

# Detect Terminal Color Support
COLORS = $(shell tput colors || echo 0)
ifeq ($(COLORS),256)
    PRESET = \033[1;32m
    RESET = \033[0m
endif

# Directory Setup
SOURCE_DIR = Sources
BUILD_DIR = Intermediates
GRAMMAR_DIR = Grammar
HEADER_DIR = Headers
META_DIR = Metadata

# Compiling RE-flex
REFLEX_DIR = Submodules/RE-flex
REFLEX_UNICODE_SRC_DIR = $(REFLEX_DIR)/unicode
REFLEX_LIB_SRC_DIR = $(REFLEX_DIR)/lib
REFLEX_LIB_HEADER_DIR = $(REFLEX_DIR)/include

REFLEX_LIB_SOURCES = $(shell find $(REFLEX_LIB_SRC_DIR) | grep .cpp)
REFLEX_UNICODE_SOURCES = $(shell find $(REFLEX_UNICODE_SRC_DIR) | grep .cpp)

REFLEX_LIB_OBJECTS = $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp,%.o,$(REFLEX_LIB_SOURCES)))
REFLEX_UNICODE_OBJECTS =  $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp,%.o,$(REFLEX_UNICODE_SOURCES)))

REFLEX_MESS = $(REFLEX_DIR)/src/reflex.cpp $(REFLEX_LIB_OBJECTS) $(REFLEX_UNICODE_OBJECTS)

# Scanner and Parser Generation
YACC_FLAGS = --verbose

LEX = $(GRAMMAR_DIR)/phi.l
YACC = $(GRAMMAR_DIR)/phi.yy
LEX_OUT = $(addprefix $(BUILD_DIR)/, $(patsubst %,%.cc,$(LEX)))
LEX_HEADER = $(addprefix $(BUILD_DIR)/, $(patsubst %,%.hh,$(LEX)))
YACC_OUT = $(addprefix $(BUILD_DIR)/, $(patsubst %,%.cc,$(YACC)))

# Localization Files
LOC_FILES = $(shell find Localization)

# Compilation
LIBRARY_HEADER_PATHS = $(addprefix -I, $(shell find Submodules -mindepth 1 -maxdepth 1)) $(addprefix -I, $(shell find Submodules -name include))

## C
C_FLAGS = -pedantic
SOURCES = 
HEADERS =
LIBRARY_SOURCES = 
OBJECTS = $(addprefix $(BUILD_DIR)/, $(patsubst %.c,%.o,$(SOURCES))) $(addprefix $(BUILD_DIR)/, $(patsubst %.c,%.o,$(LIBRARY_SOURCES)))

## C++
LLVM_INCLUDES = $(shell $(LLVM_CONFIG) --cxxflags | tr ' ' '\n' | grep '\-I' | tr '\n' ' ')
CPP_LY_FLAGS = -std=c++17 $(LLVM_INCLUDES)
CPP_FLAGS = -Wall -pedantic $(CPP_LY_FLAGS)

CPP_LY_SOURCES = $(YACC_OUT) $(LEX_OUT)
CPP_SOURCES = $(shell find Sources | grep .cpp) $(BUILD_DIR)/Localization.cpp
CPP_HEADERS = $(shell find Headers | grep .h) $(BUILD_DIR)/$(META_DIR)/git_version.h $(BUILD_DIR)/$(META_DIR)/sv_primitives.h $(LEX_HEADER)
CPP_LIBRARY_SOURCES =

CPP_LY_OBJECTS = $(patsubst %.cc,%.o,$(CPP_LY_SOURCES))
CPP_OBJECTS = $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp,%.o,$(CPP_SOURCES))) $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp,%.o,$(CPP_LIBRARY_SOURCES)))

LLVM_LD_FLAGS = $(shell $(LLVM_CONFIG) --ignore-libllvm --ldflags --system-libs) $(shell $(LLVM_CONFIG) --ignore-libllvm --libfiles support demangle)

# Final binary
BINARY = phic

# Targets
all: CPP_FLAGS += -g -DYYDEBUG=1
all: CPP_LY_FLAGS += -g -DYYDEBUG=1
all: C_FLAGS += -g
all: $(BINARY)

release: CPP_FLAGS += -O3
release: CPP_LY_FLAGS += -O3
release: C_FLAGS += -O3
release: $(BINARY)

$(BUILD_DIR)/$(META_DIR)/git_version.h:
	mkdir -p $(@D)
	echo "#ifndef _AUTO_git_version_h" > $@
	echo "#define _AUTO_git_version_h" >> $@
	echo "namespace Phi {" >> $@
	echo "namespace BuildInfo {" >> $@
	echo "const char* GIT_TAG = \"$(shell git tag | tail -n 1)\";" >> $@
	echo "const char* GIT_VER_STRING = \"$(shell git describe --always --tags)\";" >> $@
	echo "}" >> $@
	echo "}" >> $@
	echo "#endif // _AUTO_git_version_h" >> $@

$(BUILD_DIR)/$(META_DIR)/sv_primitives.h: Sources/SystemVerilog/Primitives.sv
	mkdir -p $(@D)
	echo "#ifndef _AUTO_git_version_h" > $@
	echo "#define _AUTO_git_version_h" >> $@
	echo "namespace Phi {" >> $@
	echo "namespace Common {" >> $@
	echo "const char* primitives = R\"EOF( " >> $@
	cat $< >> $@
	echo ")EOF\";" >> $@
	echo "}" >> $@
	echo "}" >> $@
	echo "#endif // _AUT0_git_version_h" >> $@

$(BUILD_DIR)/Localization.cpp: $(LOC_FILES)
	mkdir -p $(@D)
	ruby localization_into_cpp.rb > $@

$(YACC_OUT): $(YACC)
	mkdir -p $(@D)
	@echo "$(PRESET)>> Generating parser... $(RESET)"
	$(BISON) $(YACC_FLAGS) -o $@ -d $^

$(REFLEX_LIB_OBJECTS): $(BUILD_DIR)/%.o : %.cpp $(REFLEX_LIB_HEADER_DIR)
	mkdir -p $(@D)
	@echo "$(PRESET)>> Compiling $< $(RESET)"
	$(CXX) $(CPP_LY_FLAGS) -c -I$(REFLEX_LIB_HEADER_DIR) -o $@ $<

$(REFLEX_UNICODE_OBJECTS): $(BUILD_DIR)/%.o : %.cpp $(REFLEX_LIB_HEADER_DIR)
	mkdir -p $(@D)
	@echo "$(PRESET)>> Compiling $< $(RESET)"
	$(CXX) $(CPP_LY_FLAGS) -c -I$(REFLEX_LIB_HEADER_DIR) -o $@ $<

Intermediates/reflex: $(REFLEX_MESS)
	mkdir -p $(@D)
	@echo "$(PRESET)>> Compiling and linking the RE-flex binary $(RESET)"
	$(CXX) $(CPP_LY_FLAGS) -I $(REFLEX_LIB_HEADER_DIR) -o $@ $^

$(LEX_OUT): $(LEX) $(YACC_OUT) Intermediates/reflex
	mkdir -p $(@D)
	@echo "$(PRESET)>> Generating scanner... $(RESET)"
	Intermediates/reflex -o $@ --header-file=$(LEX_HEADER) $<

$(LEX_HEADER): $(LEX_OUT)
	@echo "$(PRESET)>> Scanner header generated.$(RESET)"

$(OBJECTS): $(BUILD_DIR)/%.o : %.c $(HEADERS) 
	mkdir -p $(@D)
	@echo "$(PRESET)>> Compiling $< $(RESET)"
	$(CC) $(C_FLAGS) -c -I$(HEADER_DIR) -o $@ $<

$(CPP_LY_OBJECTS): %.o : %.cc $(YACC_OUT) $(LEX_OUT) $(CPP_HEADERS) $(HEADERS)
	mkdir -p $(@D)
	@echo "$(PRESET)>> Compiling $< $(RESET)"
	$(CXX) $(CPP_LY_FLAGS) -I$(HEADER_DIR) -I$(BUILD_DIR) -I$(BUILD_DIR)/$(GRAMMAR_DIR) -I/usr/local/include/ $(LIBRARY_HEADER_PATHS) -c -o $@ $<

$(CPP_OBJECTS): $(BUILD_DIR)/%.o : %.cpp $(YACC_OUT) $(LEX_OUT) $(CPP_HEADERS) $(HEADERS)
	mkdir -p $(@D)
	@echo "$(PRESET)>> Compiling $< $(RESET)"
	$(CXX) $(CPP_FLAGS) -I$(HEADER_DIR) -I$(BUILD_DIR)/$(META_DIR) -I$(BUILD_DIR)/$(GRAMMAR_DIR) $(LIBRARY_HEADER_PATHS) -c -o $@ $<

$(BINARY): $(OBJECTS) $(CPP_OBJECTS) $(CPP_LY_OBJECTS) $(REFLEX_LIB_OBJECTS) $(REFLEX_UNICODE_OBJECTS)
	mkdir -p $(@D)
	@echo "$(PRESET)>> Linking $(BINARY) $(RESET)"
	@echo "$(PRESET)>> Using LLVM LD Flags $(LLVM_LD_FLAGS) $(RESET)"
	$(CXX) -o $@ $^ $(LLVM_LD_FLAGS)
	@echo "$(PRESET)>> Build complete.$(RESET)"

.PHONY: clean test

test: all
	@ruby ./run_tests.rb

clean_tests:
	rm -rf ./Tests/**/*.phi.sv

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(BINARY)
	rm -f Examples/*.v
	rm -f ./*.phi
	rm -f *.exe *.stackdump # MSYS2/Windows
	rm -f *.out *.phi.*sv *.dot ./*.png
