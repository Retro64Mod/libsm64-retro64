default: lib

CC      := gcc
CXX 	:= g++
CFLAGS  := -g0 -Wall -fPIC -DSM64_LIB_EXPORT -DGBI_FLOATS -DVERSION_US -DNO_SEGMENTED_MEMORY
LDFLAGS := -lm -shared -lpthread
ENDFLAGS := -fPIC

ifeq ($(shell uname),Darwin)
CFLAGS := $(CFLAGS) -I/opt/homebrew/include -I/usr/local/include -I. -Wno-error=implicit-function-declaration
else
LDFLAGS := $(LDFLAGS) -Xlinker -Map=dist/debug.map # export map file on linux and windows, macos throws "unknown option: -Map=dist/debug.map"
endif

ifeq ($(OS),Windows_NT)
LDFLAGS := $(LDFLAGS) -mwindows
ENDFLAGS := -static -lole32 -lstdc++ -limagehlp
else
LDFLAGS := $(LDFLAGS) `sdl2-config --libs` -L/usr/lib -lSDL2 # Add alsa and pulseaudio libraries for linux audio
ifneq ($(shell uname),Darwin) # Audio does not seem to be working with these flags on macosx
ENDFLAGS := $(LDFLAGS) -lasound -lpulse
endif
endif

SRC_DIRS  := src src/decomp src/decomp/engine src/decomp/include/PR src/decomp/game src/decomp/pc src/decomp/pc/audio src/decomp/mario src/decomp/tools src/decomp/audio src/decomp/model_luigi src/decomp/model_alex src/decomp/model_steve src/decomp/model_necoarc src/decomp/model_vibri
BUILD_DIR := build
DIST_DIR  := dist
ALL_DIRS  := $(addprefix $(BUILD_DIR)/,$(SRC_DIRS))

LIB_FILE   := $(DIST_DIR)/libsm64.so
LIB_H_FILE := $(DIST_DIR)/include/libsm64.h
TEST_FILE  := run-test

H_IMPORTED := $(C_IMPORTED:.c=.h)
IMPORTED   := $(C_IMPORTED) $(H_IMPORTED)

C_FILES   := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c)) $(C_IMPORTED)
CXX_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))
O_FILES   := $(foreach file,$(C_FILES),$(BUILD_DIR)/$(file:.c=.o)) $(foreach file,$(CXX_FILES),$(BUILD_DIR)/$(file:.cpp=.o))

DEP_FILES := $(O_FILES:.o=.d)

TEST_SRCS := test/main.c test/context.c test/level.c
TEST_OBJS := $(foreach file,$(TEST_SRCS),$(BUILD_DIR)/$(file:.c=.o))

ifeq ($(OS),Windows_NT)
  TEST_FILE := $(DIST_DIR)/$(TEST_FILE)
  LIB_FILE := $(DIST_DIR)/sm64.dll
else ifeq ($(shell uname),Darwin)
  TEST_FILE := $(DIST_DIR)/$(TEST_FILE)
  LIB_FILE := $(DIST_DIR)/libsm64.dylib
endif

DUMMY != mkdir -p $(ALL_DIRS) build/test src/decomp/mario $(DIST_DIR)/include 

$(BUILD_DIR)/%.o: %.c $(IMPORTED)
	@$(CC) $(CFLAGS) -MM -MP -MT $@ -MF $(BUILD_DIR)/$*.d $<
	$(CC) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: %.cpp $(IMPORTED)
	@$(CXX) $(CFLAGS) -MM -MP -MT $@ -MF $(BUILD_DIR)/$*.d $<
	$(CXX) -c $(CFLAGS) -o $@ $<

$(LIB_FILE): $(O_FILES)
	$(CC) $(LDFLAGS) -o $@ $^ $(ENDFLAGS)

$(LIB_H_FILE): src/libsm64.h
	cp -f $< $@


test/level.c test/level.h: ./import-test-collision.py
	./import-test-collision.py

test/main.c: test/level.h

$(BUILD_DIR)/test/%.o: test/%.c
	@$(CC) $(CFLAGS) -MM -MP -MT $@ -MF $(BUILD_DIR)/test/$*.d $<
ifeq ($(OS),Windows_NT)
	$(CC) -c $(CFLAGS) -I/mingw64/include/SDL2 -I/mingw64/include/GL -o $@ $<
else
	$(CC) -c $(CFLAGS) -o $@ $<
endif

$(TEST_FILE): $(LIB_FILE) $(TEST_OBJS)
ifeq ($(OS),Windows_NT)
	$(CC) -o $@ $(TEST_OBJS) $(LIB_FILE) `sdl2-config --cflags --libs` -lglew32 -lglu32 -lopengl32 -lSDL2 -lSDL2main -lm
else
	$(CC) -o $@ $(TEST_OBJS) $(LIB_FILE) -lGLEW -lGL -lSDL2 -lSDL2main -lm
endif


lib: $(LIB_FILE) $(LIB_H_FILE)

test: $(TEST_FILE) $(LIB_H_FILE)

run: test
	./$(TEST_FILE)

clean:
	rm -rf $(BUILD_DIR) $(DIST_DIR) test/level.? $(TEST_FILE)

-include $(DEP_FILES)
