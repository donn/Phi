export POSIXLY_CORRECT = 1

SOURCE_DIR = Sources
BUILD_DIR = Intermediates

LEX = $(SOURCE_DIR)/phi.l
YACC = $(SOURCE_DIR)/phi.y
LEX_OUT = $(addprefix $(BUILD_DIR)/, $(patsubst %,%.c,$(LEX)))
LEX_HEADER = $(addprefix $(BUILD_DIR)/, $(patsubst %,%.h,$(LEX)))
YACC_OUT = $(addprefix $(BUILD_DIR)/, $(patsubst %,%.c,$(YACC)))

YACC_FLAGS = --verbose
C_FLAGS = -pedantic
CPP_FLAGS = -pedantic -std=c++17

SOURCES = $(SOURCE_DIR)/Node.c
HEADERS = $(SOURCE_DIR)/Node.h $(BUILD_DIR)/$(SOURCE_DIR)/git_version.h

CPP_SOURCES = $(SOURCE_DIR)/Errors.cpp $(SOURCE_DIR)/main.cpp 
CPP_HEADERS = $(SOURCE_DIR)/Errors.h

LIBRARY_HEADER_PATHS = $(addprefix -I, $(shell find Submodules -depth 1))
LIBRARY_SOURCES = 
CPP_LIBRARY_SOURCES = 

LY_SOURCES = $(LEX_OUT) $(YACC_OUT)

OBJECTS = $(addprefix $(BUILD_DIR)/, $(patsubst %.c,%.o,$(SOURCES))) $(addprefix $(BUILD_DIR)/, $(patsubst %.c,%.o,$(LIBRARY_SOURCES)))
CPP_OBJECTS = $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp,%.o,$(CPP_SOURCES))) $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp,%.o,$(CPP_LIBRARY_SOURCES)))
LY_OBJECTS = $(patsubst %.c,%.o,$(LY_SOURCES))

BINARY = phi

# Products

all: CPP_FLAGS += -g -D_DEBUG
all: C_FLAGS += -g -D_DEBUG -DYY_DEBUG=1
all: $(BINARY)

deep: YACC_FLAGS += -t
deep: all

release: $(BINARY)

$(BUILD_DIR)/$(SOURCE_DIR)/git_version.h:
	mkdir -p $(@D)
	echo "#ifndef _AUTO_git_version_h" > $@
	echo "#define _AUTO_git_version_h" >> $@
	echo "#define GIT_TAG \"$(shell git tag)\"" >> $@
	echo "#define GIT_VER_STRING \"$(shell git describe --always --tags)\"" >> $@
	echo "#endif // _AUT0_git_version_h" >> $@

$(YACC_OUT): $(YACC)
	mkdir -p $(@D)
	yacc $(YACC_FLAGS) -o $@ -d $^

$(LEX_OUT): $(LEX) $(YACC_OUT)
	mkdir -p $(@D)
	lex --header-file=$(LEX_HEADER) -o $@ $<

$(OBJECTS): $(BUILD_DIR)/%.o : %.c $(HEADERS)
	mkdir -p $(@D)
	cc $(C_FLAGS) -c -ISources -o $@ $<

$(CPP_OBJECTS): $(BUILD_DIR)/%.o : %.cpp $(YACC_OUT) $(LEX_OUT) $(CPP_HEADERS) $(HEADERS)
	mkdir -p $(@D)
	c++ $(CPP_FLAGS) -I$(BUILD_DIR)/$(SOURCE_DIR) $(LIBRARY_HEADER_PATHS) -c -o $@ $<

$(LY_OBJECTS): %.o : %.c $(HEADERS)
	mkdir -p $(@D)
	cc $(C_FLAGS) -c -ISources -o $@ $<

$(BINARY): $(OBJECTS) $(CPP_OBJECTS) $(LY_OBJECTS)
	mkdir -p $(@D)
	c++ -o $@ $^
	@echo "\033[1;32m>> Build complete.\033[0m"

.PHONY: clean

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(BINARY)