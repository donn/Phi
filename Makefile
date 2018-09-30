SOURCE_DIR = Sources
BUILD_DIR = Intermediates

LEX = $(SOURCE_DIR)/phi.l
YACC = $(SOURCE_DIR)/phi.y
LEX_OUT = $(addprefix $(BUILD_DIR)/, $(patsubst %,%.c,$(LEX)))
LEX_HEADER = $(addprefix $(BUILD_DIR)/, $(patsubst %,%.h,$(LEX)))
YACC_OUT = $(addprefix $(BUILD_DIR)/, $(patsubst %,%.c,$(YACC)))

C_FLAGS = 
CPP_FLAGS = -pedantic -std=c++14

SOURCES = $(SOURCE_DIR)/Node.c
HEADERS = $(SOURCE_DIR)/Node.h

CPP_SOURCES = $(SOURCE_DIR)/main.cpp
CPP_HEADERS = 

LY_SOURCES = $(LEX_OUT) $(YACC_OUT)

OBJECTS = $(addprefix $(BUILD_DIR)/, $(patsubst %.c,%.o,$(SOURCES)))
CPP_OBJECTS = $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp,%.o,$(CPP_SOURCES)))
LY_OBJECTS = $(patsubst %.c,%.o,$(LY_SOURCES))

BINARY = phi

# Products

all: CPP_FLAGS += -g -D _DEBUG
all: C_FLAGS += -g -D _DEBUG
all: $(BINARY)

release: $(BINARY)

$(YACC_OUT): $(YACC)
	mkdir -p $(@D)
	yacc --verbose -o $@ -d $^

$(LEX_OUT): $(LEX) $(YACC_OUT)
	mkdir -p $(@D)
	lex --header-file=$(LEX_HEADER) -o $@ $<

$(OBJECTS): $(BUILD_DIR)/%.o : %.c $(HEADERS)
	mkdir -p $(@D)
	cc $(C_FLAGS) -c -ISources -o $@ $<

$(CPP_OBJECTS): $(BUILD_DIR)/%.o : %.cpp $(YACC_OUT) $(LEX_OUT)
	mkdir -p $(@D)
	c++ $(CPP_FLAGS) -I$(BUILD_DIR)/$(SOURCE_DIR) -c -o $@ $<

$(LY_OBJECTS): %.o : %.c $(HEADERS)
	mkdir -p $(@D)
	cc -c -ISources -o $@ $<

$(BINARY): $(OBJECTS) $(CPP_OBJECTS) $(LY_OBJECTS)
	mkdir -p $(@D)
	c++ -o $@ $^
	@echo "\033[1;32m>> Build complete.\033[0m"

.PHONY: clean

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(BINARY)