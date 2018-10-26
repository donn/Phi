export POSIXLY_CORRECT = 1

SOURCE_DIR = Sources
BUILD_DIR = Intermediates
GRAMMAR_DIR = Grammar
HEADER_DIR = Headers

LEX = $(GRAMMAR_DIR)/phi.l
YACC = $(GRAMMAR_DIR)/phi.yy
LEX_OUT = $(addprefix $(BUILD_DIR)/, $(patsubst %,%.cc,$(LEX)))
LEX_HEADER = $(addprefix $(BUILD_DIR)/, $(patsubst %,%.hh,$(LEX)))
YACC_OUT = $(addprefix $(BUILD_DIR)/, $(patsubst %,%.cc,$(YACC)))

YACC_FLAGS = --verbose
C_FLAGS = -pedantic
CPP_LY_FLAGS = -Wno-deprecated-register -std=c++11
CPP_FLAGS = -pedantic -std=c++17

SOURCES = 
HEADERS =

CPP_LY_SOURCES = $(YACC_OUT) $(LEX_OUT)
CPP_SOURCES = $(shell find Sources -depth 1)
CPP_HEADERS = $(shell find Headers -depth 1) $(BUILD_DIR)/git_version.h  $(LEX_HEADER)

LIBRARY_HEADER_PATHS = $(addprefix -I, $(shell find Submodules -mindepth 1 -maxdepth 1))
LIBRARY_SOURCES = 
CPP_LIBRARY_SOURCES =

OBJECTS = $(addprefix $(BUILD_DIR)/, $(patsubst %.c,%.o,$(SOURCES))) $(addprefix $(BUILD_DIR)/, $(patsubst %.c,%.o,$(LIBRARY_SOURCES)))

CPP_LY_OBJECTS = $(patsubst %.cc,%.o,$(CPP_LY_SOURCES))
CPP_OBJECTS = $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp,%.o,$(CPP_SOURCES))) $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp,%.o,$(CPP_LIBRARY_SOURCES)))

BINARY = phi

# Products

all: CPP_FLAGS += -g -D_DEBUG
all: C_FLAGS += -g -D_DEBUG -DYY_DEBUG=1
all: $(BINARY)

deep: YACC_FLAGS += -t
deep: all

release: -O3
release: $(BINARY)

$(BUILD_DIR)/git_version.h:
	mkdir -p $(@D)
	echo "#ifndef _AUTO_git_version_h" > $@
	echo "#define _AUTO_git_version_h" >> $@
	echo "namespace Phi {" >> $@
	echo "const char* GIT_TAG = \"$(shell git tag)\";" >> $@
	echo "const char* GIT_VER_STRING = \"$(shell git describe --always --tags)\";" >> $@
	echo "}" >> $@
	echo "#endif // _AUT0_git_version_h" >> $@

$(YACC_OUT): $(YACC)
	mkdir -p $(@D)
	bison $(YACC_FLAGS) -o $@ -d $^

$(LEX_OUT): $(LEX) $(YACC_OUT)
	mkdir -p $(@D)
	flex --header-file=$(LEX_HEADER) -o $@ $<

$(LEX_HEADER): $(LEX_OUT)
	echo "Header generated."

$(OBJECTS): $(BUILD_DIR)/%.o : %.c $(HEADERS)
	mkdir -p $(@D)
	cc $(C_FLAGS) -c -I$(HEADER_DIR) -o $@ $<

$(CPP_LY_OBJECTS): %.o : %.cc $(YACC_OUT) $(LEX_OUT) $(CPP_HEADERS) $(HEADERS)
	mkdir -p $(@D)
	c++ $(CPP_LY_FLAGS) -I$(HEADER_DIR) -I$(BUILD_DIR) -I$(BUILD_DIR)/$(GRAMMAR_DIR) $(LIBRARY_HEADER_PATHS) -c -o $@ $<

$(CPP_OBJECTS): $(BUILD_DIR)/%.o : %.cpp $(YACC_OUT) $(LEX_OUT) $(CPP_HEADERS) $(HEADERS)
	mkdir -p $(@D)
	c++ $(CPP_FLAGS) -I$(HEADER_DIR) -I$(BUILD_DIR) -I$(BUILD_DIR)/$(GRAMMAR_DIR) $(LIBRARY_HEADER_PATHS) -c -o $@ $<

$(BINARY): $(OBJECTS) $(CPP_OBJECTS) $(CPP_LY_OBJECTS)
	mkdir -p $(@D)
	echo $(CPP_LY_OBJECTS)
	c++ -o $@ $^
	@echo "\033[1;32m>> Build complete.\033[0m"

.PHONY: clean

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(BINARY)
