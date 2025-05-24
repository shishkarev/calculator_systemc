CXX ?= clang++
CXXFLAGS = -Wall -Wextra -pedantic -O2 -std=c++17 -I./include
LDLIBS = -lsystemc

# Detect SystemC installation
SYSTEMC_HOME ?= $(shell pkg-config systemc --variable=prefix 2>/dev/null)
ifeq ($(SYSTEMC_HOME),)
    # Fallback to common installation paths
    ifneq ($(wildcard /usr/local/systemc-*),)
        SYSTEMC_HOME = $(wildcard /usr/local/systemc-*)
    else ifneq ($(wildcard /opt/systemc-*),)
        SYSTEMC_HOME = $(wildcard /opt/systemc-*)
    else ifneq ($(wildcard /opt/homebrew/Cellar/systemc/*),)
        SYSTEMC_HOME = $(wildcard /opt/homebrew/Cellar/systemc/*)
    endif
endif

ifneq ($(SYSTEMC_HOME),)
    CXXFLAGS += -I$(SYSTEMC_HOME)/include
    LDFLAGS += -L$(SYSTEMC_HOME)/lib -Wl,-rpath,$(SYSTEMC_HOME)/lib
else
    $(warning SystemC installation not found. Trying default paths.)
    CXXFLAGS += $(shell pkg-config --cflags systemc 2>/dev/null)
    LDFLAGS += $(shell pkg-config --libs-only-L --libs-only-other systemc 2>/dev/null)
    LDLIBS += $(shell pkg-config --libs-only-l systemc 2>/dev/null)
endif

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))
TARGET = $(BIN_DIR)/simple_calculator_pipeline

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)