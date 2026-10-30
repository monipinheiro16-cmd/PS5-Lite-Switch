
TARGET      := ps5lite
BUILD       := build
SOURCES     := .
DATA        :=
INCLUDES    :=

APP_TITLE   := PS5 Lite AI
APP_AUTHOR  := PS5 Lite Project
APP_VERSION := 0.2.0

CFLAGS      := -g -Wall -O2 -ffunction-sections -fdata-sections
CXXFLAGS    := $(CFLAGS) -fno-rtti -fno-exceptions
ASFLAGS     := -g
LDFLAGS     = -specs=$(DEVKITPRO)/libnx/switch.specs -g
LIBS        := -lnx

include $(DEVKITPRO)/libnx/switch_rules
