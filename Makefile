
# Nome do aplicativo
TARGET := switchps5lite

# Diretorios
BUILD := build
SOURCES := source
DATA := data
INCLUDES := include

# Informacoes do aplicativo
APP_TITLE := PS5 Lite Switch
APP_AUTHOR := Homebrew Project
APP_VERSION := 1.0.0

# Configuracao do Nintendo Switch
include $(DEVKITPRO)/libnx/switch_rules
