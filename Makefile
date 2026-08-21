# Makefile of the Newfon speech core
#
# Builds the core as a static library and, optionally, a small command
# line tool that writes a wave file (make newfon_speak).

ARCH = x64
TARGET = x86_64-w64-mingw32
CC = clang

ifeq ($(ARCH), x86)
TARGET = i686-w64-mingw32
endif

TARGET_CC = $(TARGET)-$(CC)
BUILD_DIR = build/$(ARCH)

CFLAGS = -Wall -Wextra -Werror -Wno-unused-parameter -Wno-sign-compare -O3 -Iinclude -Isrc
OBJS = female.o intonator.o male.o numerics.o sink.o soundproducer.o \
       speechrate_control.o synth.o text2speech.o time_planner.o \
       transcription.o utterance.o voices.o
TARGET_OBJS = $(addprefix $(BUILD_DIR)/, $(OBJS))
LIB = $(BUILD_DIR)/libnewfon_core.a

all: $(LIB)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(TARGET_CC) $(CFLAGS) -o $@ -c $<

$(LIB): $(TARGET_OBJS)
	$(TARGET)-llvm-ar rcs $@ $^

newfon_speak: $(BUILD_DIR)/newfon_speak.exe

$(BUILD_DIR)/newfon_speak.exe: tools/newfon_speak.c $(LIB)
	$(TARGET_CC) $(CFLAGS) -o $@ $< $(LIB)

.PHONY .SILENT: clean
clean:
	rm -rf build

$(BUILD_DIR)/female.o: src/female.c src/voice.h
$(BUILD_DIR)/male.o: src/male.c src/voice.h
$(BUILD_DIR)/voices.o: src/voices.c src/voice.h
$(BUILD_DIR)/intonator.o: src/intonator.c src/transcription.h src/soundscript.h src/modulation.h src/voice.h
$(BUILD_DIR)/numerics.o: src/numerics.c src/numerics.h src/transcription.h src/sink.h src/synth.h
$(BUILD_DIR)/sink.o: src/sink.c src/sink.h
$(BUILD_DIR)/soundproducer.o: src/soundproducer.c src/sink.h src/soundscript.h src/voice.h
$(BUILD_DIR)/speechrate_control.o: src/speechrate_control.c src/timing.h src/soundscript.h src/transcription.h
$(BUILD_DIR)/synth.o: src/synth.c src/synth.h src/transcription.h src/soundscript.h src/voice.h
$(BUILD_DIR)/text2speech.o: src/text2speech.c src/sink.h src/transcription.h src/synth.h src/timing.h src/modulation.h src/voice.h
$(BUILD_DIR)/time_planner.o: src/time_planner.c src/soundscript.h src/transcription.h
$(BUILD_DIR)/transcription.o: src/transcription.c src/transcription.h src/numerics.h src/sink.h
$(BUILD_DIR)/utterance.o: src/utterance.c src/soundscript.h src/transcription.h
