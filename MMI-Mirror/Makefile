# MHI2Q MMI Mirror V2.2 / JAVA80 pixel-only Native build

BUILD_DIR ?= build
TARGET ?= $(BUILD_DIR)/mmi-mirror-display

QNX_HOST ?= /usr/qnx650/host/qnx6/x86
QNX_TARGET ?= /usr/qnx650/target/qnx6
CC ?= $(QNX_HOST)/usr/bin/ntoarmv7-gcc
STATIC_STDCXX ?= $(QNX_TARGET)/armle-v7/lib/gcc/4.4.2/libstdc++.a

SOURCES = \
	src/main_v2.cpp \
	src/v2_options.cpp \
	src/v2_runtime.cpp \
	src/mmi_capture_source.cpp \
	src/cluster_layout_state.cpp \
	src/base_video_layout_controller.cpp \
	src/cluster_video_display.cpp \
	src/mhi2q_backend.cpp \
	src/gl_renderer.cpp

CXXFLAGS += -O2 -Wall -Wextra -fno-exceptions -fno-rtti
LDFLAGS += -lEGL -lGLESv2 -lm

.PHONY: all clean test
all: $(TARGET)
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
$(TARGET): $(SOURCES) | $(BUILD_DIR)
	@test -f "$(STATIC_STDCXX)" || { echo "ERROR: static QNX ARMv7 libstdc++ not found: $(STATIC_STDCXX)"; exit 1; }
	QNX_HOST="$(QNX_HOST)" QNX_TARGET="$(QNX_TARGET)" "$(CC)" $(CXXFLAGS) -Isrc $(SOURCES) -o "$@" "$(STATIC_STDCXX)" $(LDFLAGS)
	@echo "Built: $@"
clean:
	rm -rf $(BUILD_DIR)

# Host-only geometry/renderer tests: no QNX SDK, EGL library or GPU required.
HOST_CXX ?= c++
HOST_CXXFLAGS ?= -O2 -Wall -Wextra -std=c++98
HOST_TEST_DIR = $(BUILD_DIR)/host
HOST_TEST = $(HOST_TEST_DIR)/adaptive_geometry_tests
HOST_TEST_SOURCES = tests/adaptive_geometry_tests.cpp tests/fake_gl.cpp \
	src/base_video_layout_controller.cpp src/cluster_layout_state.cpp \
	src/v2_options.cpp src/gl_renderer.cpp
HOST_TEST_HEADERS = $(wildcard src/*.h tests/*.h tests/stubs/EGL/*.h tests/stubs/GLES2/*.h)

$(HOST_TEST_DIR):
	mkdir -p "$@"
$(HOST_TEST): $(HOST_TEST_SOURCES) $(HOST_TEST_HEADERS) | $(HOST_TEST_DIR)
	$(HOST_CXX) $(HOST_CXXFLAGS) -Itests/stubs -Itests -Isrc $(HOST_TEST_SOURCES) -o "$@"
test: $(HOST_TEST)
	"$(HOST_TEST)"
