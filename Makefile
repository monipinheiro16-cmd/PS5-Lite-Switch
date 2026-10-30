
TARGET := switchps5lite
BUILD := build
SOURCES := source
DATA :=
INCLUDES :=

APP_TITLE := PS5 Lite Switch
APP_AUTHOR := Homebrew Project
APP_VERSION := 1.0.0

CFLAGS := -O2 -Wall -D__SWITCH__
CXXFLAGS := $(CFLAGS)

include $(DEVKITPRO)/libnx/switch_rules

.PHONY: all
all: $(TARGET).nro
