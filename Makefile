
TARGET := PS5-Lite-Switch
BUILD := build
SOURCES := source

CXX := $(DEVKITPRO)/devkitA64/bin/aarch64-none-elf-g++

CXXFILES := $(wildcard $(SOURCES)/*.cpp)
OBJECTS := $(patsubst $(SOURCES)/%.cpp,$(BUILD)/%.o,$(CXXFILES))

CXXFLAGS := -O2 -Wall -std=gnu++17 \
-march=armv8-a+crc+crypto \
-mtune=cortex-a57 -mtp=soft -fPIE

LDFLAGS := -specs=$(DEVKITPRO)/libnx/switch.specs \
-march=armv8-a+crc+crypto \
-mtune=cortex-a57 -mtp=soft -fPIE

all: $(TARGET).nro

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: $(SOURCES)/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJECTS)
	$(CXX) $(LDFLAGS) $^ -lnx -o $@

$(TARGET).nro: $(TARGET).elf
	$(DEVKITPRO)/tools/bin/elf2nro $< $@

clean:
	rm -rf $(BUILD) $(TARGET).elf $(TARGET).nro
