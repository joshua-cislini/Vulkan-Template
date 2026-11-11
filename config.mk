# =============================================================================
#  Project Configuration (config.mk) — Vulkan C++23 Starter Template
# =============================================================================
#  This file centralizes all project-level settings for your Vulkan application.
#  Customize the project name, target type, C++ compiler flags, and dependencies
#  here. The Makefile automatically reads these settings and discovers your source
#  files, shaders, and libraries so you can focus directly on graphics code.
#
#  Every variable uses `?=` so it can also be overridden from the command line:
#      make PROJECT=my-vulkan-app BUILD=release
# =============================================================================

# --- Identity ----------------------------------------------------------------

# Name of the produced binary. Defaults to the name of the project folder.
PROJECT ?= $(PROJECT_DIR_NAME)

# Exposed to the code as the PROJECT_VERSION string macro.
VERSION ?= 0.1.0

# What to build (executable for Vulkan applications).
PROJECT_TYPE ?= executable

# Default build mode: debug or release (`make release` is a shortcut).
# Debug enables Vulkan validation layers (VK_LAYER_KHRONOS_validation).
# Release disables validation layers and applies -O2 optimizations.
BUILD ?= debug

# --- Toolchain ---------------------------------------------------------------

# C++ compiler (g++, clang++, x86_64-w64-mingw32-g++, ...)
ifeq ($(origin CXX),default)
CXX := g++
endif

# --- Language and warnings ---------------------------------------------------

# C++ standard: c++23, c++20, c++17, ...
CXX_STANDARD ?= c++23

WARNINGS ?= -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -Wundef \
            -Wnon-virtual-dtor -Woverloaded-virtual -Wno-missing-field-initializers

# Set to 1 to turn every warning into an error.
WERROR ?= 0

# --- Per-mode flags ----------------------------------------------------------

DEBUG_CXXFLAGS   ?= -g3 -O0 -DDEBUG
RELEASE_CXXFLAGS ?= -O2 -DNDEBUG

DEBUG_LDFLAGS   ?=
RELEASE_LDFLAGS ?= -s

# Sanitizers, e.g. `make SANITIZE=address,undefined` (Linux / macOS).
SANITIZE ?=

# --- Extra flags (appended to defaults) ---------------------------------------

# If the LunarG Vulkan SDK environment variable is present, link its headers/libs
ifneq ($(strip $(VULKAN_SDK)),)
  EXTRA_CPPFLAGS += -I"$(VULKAN_SDK)/include"
  ifeq ($(OS),Windows_NT)
    EXTRA_LDFLAGS += -L"$(VULKAN_SDK)/Lib"
  else
    EXTRA_LDFLAGS += -L"$(VULKAN_SDK)/lib"
  endif
endif

# macOS Homebrew include and library paths
ifneq ($(OS),Windows_NT)
ifeq ($(shell uname -s 2>/dev/null),Darwin)
  EXTRA_CPPFLAGS += -I/opt/homebrew/include -I/usr/local/include
  EXTRA_LDFLAGS  += -L/opt/homebrew/lib -L/usr/local/lib
endif
endif

EXTRA_CPPFLAGS ?=
EXTRA_CXXFLAGS ?=
EXTRA_LDFLAGS  ?=

# pkg-config packages to compile and link against (Vulkan and GLFW)
PKG_CONFIG_PACKAGES ?= vulkan glfw3

# System libraries (used as fallback when pkg-config packages are not found)
ifeq ($(OS),Windows_NT)
  LDLIBS ?= -lvulkan-1 -lglfw3 -lgdi32
else ifeq ($(shell uname -s 2>/dev/null),Darwin)
  LDLIBS ?= -lvulkan -lglfw -framework Cocoa -framework IOKit -framework CoreVideo
else
  LDLIBS ?= -lvulkan -lglfw -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
endif

# --- Drop-in directories & layout --------------------------------------------

# Project directories (all scanned recursively by the Makefile):
#   src/     -> C++ source files (.cpp) and internal headers (.hpp/.h)
#   include/ -> Public & third-party header files (.hpp/.h)
#   shaders/ -> GLSL shaders (.vert, .frag, .comp, ...) compiled by glslc to SPIR-V
#   lib/     -> Third-party static & import libraries (.a, .lib, .dll.a, .so, .dylib)
#   bin/     -> Third-party runtime shared libraries (.dll, .so, .dylib) copied to build/<mode>/bin/
#   tests/   -> Unit tests (test_*.cpp)
SRC_DIR    ?= src
INC_DIR    ?= include
SHADER_DIR ?= shaders
LIB_DIR    ?= lib
BIN_DIR    ?= bin
TEST_DIR   ?= tests
BUILD_DIR  ?= build

# Libraries in lib/ are linked automatically when AUTO_LINK is 1
AUTO_LINK ?= 1
AUTO_LINK_EXCLUDE ?=

# The source file holding main()
MAIN_SRC ?= $(SRC_DIR)/main.cpp

# --- Tooling -----------------------------------------------------------------

# Shader compiler (glslc is part of the Vulkan SDK and MSYS2 shaderc)
GLSLC ?= glslc

# Generate compile_commands.json for clangd / clang-tidy on every make run
COMPILE_COMMANDS ?= 1

CLANG_FORMAT ?= clang-format
CLANG_TIDY   ?= clang-tidy
PKG_CONFIG   ?= pkg-config

# Arguments passed to the program by `make run`
ARGS ?=
