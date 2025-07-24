# Simple Makefile for Brownian Motion Simulation
# Alternative to CMake for quick building

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

# SFML flags - adjust paths if needed
SFML_FLAGS = -lsfml-graphics -lsfml-window -lsfml-system

# Platform-specific settings
ifeq ($(shell uname), Darwin)
    BREW_PREFIX = $(shell brew --prefix 2>/dev/null || echo /opt/homebrew)
    INCLUDE_FLAGS = -I$(BREW_PREFIX)/include
    LDFLAGS += -L$(BREW_PREFIX)/lib
    
    # ARM NEON optimizations for Apple Silicon
    ifeq ($(shell uname -m), arm64)
        ARCH_FLAGS = -march=armv8-a+simd -mtune=native
    endif
else
    # x86 SSE optimizations for Intel/AMD
    ARCH_FLAGS = -msse2 -msse3 -msse4.1
endif

# Source files
SRCDIR = src
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

# Output
TARGET = brownian_simulation

# Build modes for demo
BASE_FLAGS = -O3 -DNDEBUG -ffast-math -funroll-loops
SLOW_FLAGS = $(BASE_FLAGS) -DUSE_SLOW_MATRIX
FAST_FLAGS = $(BASE_FLAGS) -DUSE_FAST_MATRIX

# Ultra flags with architecture-specific SIMD optimizations
ifeq ($(shell uname -m), arm64)
    ULTRA_FLAGS = $(BASE_FLAGS) -DUSE_ULTRA_FAST_MATRIX -mcpu=native
else
    ULTRA_FLAGS = $(BASE_FLAGS) -DUSE_ULTRA_FAST_MATRIX -mavx2 -mfma
endif  

# Default compiler flags (use slow version by default)
CXXFLAGS = -std=c++17 -Wall -Wextra -g $(INCLUDE_FLAGS) $(ARCH_FLAGS) $(SLOW_FLAGS)

.PHONY: all clean slow fast ultra install-deps help

all: slow

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS) $(SFML_FLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

slow: clean
	@echo "Building SLOW version (for demonstration)..."
	$(MAKE) $(TARGET) CXXFLAGS="-std=c++17 -Wall -Wextra -g $(INCLUDE_FLAGS) $(ARCH_FLAGS) $(SLOW_FLAGS)"

fast: clean  
	@echo "Building FAST version (cache-optimized)..."
	$(MAKE) $(TARGET) CXXFLAGS="-std=c++17 -Wall -Wextra -g $(INCLUDE_FLAGS) $(ARCH_FLAGS) $(FAST_FLAGS)"

ultra: clean
	@echo "Building ULTRA FAST version (SIMD + cache-optimized)..."
	$(MAKE) $(TARGET) CXXFLAGS="-std=c++17 -Wall -Wextra -g $(INCLUDE_FLAGS) $(ARCH_FLAGS) $(ULTRA_FLAGS)"

clean:
	rm -f $(OBJECTS) $(TARGET)

# Install dependencies
install-deps:
	@echo "Installing dependencies..."
ifeq ($(shell uname), Darwin)
	@echo "On macOS - installing via Homebrew..."
	brew install sfml
else ifeq ($(shell uname), Linux)
	@echo "On Linux - use your package manager:"
	@echo "Ubuntu/Debian: sudo apt-get install libsfml-dev"
	@echo "Fedora: sudo dnf install SFML-devel"
	@echo "Arch: sudo pacman -S sfml"
else
	@echo "Unknown OS - please install SFML manually"
endif

# Run the program
run: $(TARGET)
	./$(TARGET)

# Help
help:
	@echo "🚀 Brownian Motion Performance Demo - Build Targets:"
	@echo ""
	@echo "  slow      - Build SLOW version (~15 FPS) - shows the problem"
	@echo "  fast      - Build FAST version (~70 FPS) - cache optimization"
	@echo "  ultra     - Build ULTRA FAST version (~120+ FPS) - SIMD + cache optimization"
	@echo ""
	@echo "  clean        - Remove build artifacts"
	@echo "  install-deps - Install SFML dependencies"
	@echo "  run          - Build and run the program"
	@echo "  help         - Show this help"
	@echo ""
	@echo "🖥️  Usage:"
	@echo "  ./brownian_simulation              - Run with graphics"
	@echo "  ./brownian_simulation -no-visualize - Run headless mode (Ctrl+C to stop)"
	@echo ""