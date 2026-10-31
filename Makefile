ifeq ($(strip $(DEVKITPRO)),)
$(error "Defina DEVKITPRO (ex.: export DEVKITPRO=/opt/devkitpro)")
endif

TARGET  := PS5-Lite-Switch
BUILD   := build
SOURCES := source

PREFIX := $(DEVKITPRO)/devkitA64/bin/aarch64-none-elf-
CC     := $(PREFIX)gcc
CXX    := $(PREFIX)g++

LIBNX := $(DEVKITPRO)/libnx

ARCH     := -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE
INCLUDES := -I$(SOURCES) -I$(LIBNX)/include
DEFINES  := -D__SWITCH__
COMMON   := -O2 -Wall -ffunction-sections $(ARCH) $(DEFINES) $(INCLUDES)

CFLAGS   := $(COMMON) -std=gnu11
CXXFLAGS := $(COMMON) -fno-rtti -fno-exceptions -std=gnu++17
LDFLAGS  := -specs=$(LIBNX)/switch.specs $(ARCH) -Wl,-Map,$(BUILD)/$(TARGET).map
LIBS     := -L$(LIBNX)/lib -lnx

CFILES   := $(wildcard $(SOURCES)/*.c)
CXXFILES := $(wildcard $(SOURCES)/*.cpp)
OBJECTS  := $(patsubst $(SOURCES)/%.c,$(BUILD)/%.o,$(CFILES)) \
            $(patsubst $(SOURCES)/%.cpp,$(BUILD)/%.o,$(CXXFILES))
DEPS     := $(OBJECTS:.o=.d)

.PHONY: all clean

all: $(TARGET).nro

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: $(SOURCES)/%.c | $(BUILD)
	$(CC) $(CFLAGS) -MMD -MP -MF $(@:.o=.d) -c $< -o $@

$(BUILD)/%.o: $(SOURCES)/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -MMD -MP -MF $(@:.o=.d) -c $< -o $@

$(TARGET).elf: $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

$(TARGET).nro: $(TARGET).elf
	$(DEVKITPRO)/tools/bin/elf2nro $< $@

clean:
	rm -rf $(BUILD) $(TARGET).elf $(TARGET).nro

-include $(DEPS)
