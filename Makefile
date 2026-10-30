TARGET := ps5lite
BUILD := build
SOURCES := source
DATA :=
INCLUDES :=

APP_TITLE := PS5 Lite Switch
APP_AUTHOR := Homebrew
APP_VERSION := 1.0.0

CFLAGS := -g -Wall -O2 -ffunction-sections -fdata-sections
CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions
ASFLAGS := -g
LDFLAGS = -specs=$(DEVKITPRO)/libnx/switch.specs -g
LIBS := -lnx

include $(DEVKITPRO)/libnx/switch_rules
