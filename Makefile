EXE = GAH

//SRC_FILES = $(wildcard src/*.cpp) $(wildcard src/imGui/*.cpp) $(wildcard src/libs/gl3w/GL/*.c)
BUILD_DIR ?= build
SRC_DIR ?= src
SRC_FILES := $(shell find $(SRC_DIR) -name *.cpp -or -name *.c)
//OBJ_FILES := $(SRC_FILES:%=$(BUILD_DIR)/%.o)
OBJS = $(addsuffix .o, $(basename $(SRC_FILES)))
OBJ_FILES = $(OBJS:$(SRC_DIR)/%=$(BUILD_DIR)/%)
UNAME_S := $(shell uname -s)

ECHO_MESSAGE = "Linux"
CXXFLAGS += -g -Wall -Wformat \
			-I $(SRC_DIR) -I ./$(BUILD_DIR) -I ./src/imGui -I ./src/libs -I ./src/libs/gl3w -I ./src/libs/gl3w/GL -I ./src/libs/glfw/include \
			-I ./src/libs/usynergy -I ./src/libs/ogl -DIMGUI_IMPL_OPENGL_LOADER_GL3W `pkg-config --cflags glfw3`

LIBS = -lGL -lGLU -lGLEW `pkg-config --static --libs glfw3`
CFLAGS = $(CXXFLAGS)

ifeq ($(os), win)
	EXE = GAH.exe

	CC=x86_64-w64-mingw32-gcc
	CXX=x86_64-w64-mingw32-g++

	ECHO_MESSAGE = "Windows"
	LIBS = -lglfw3 -lgdi32 -lopengl32 -limm32 -lglew32 -lm

	CXXFLAGS = -g -Wall -Wformat \
			-I ./src -I ./$(BUILD_DIR) -I ./src/imGui -I ./src/libs -I ./src/libs/gl3w -I ./src/libs/gl3w/GL -I ./src/libs/glfw/include \
			-I ./src/libs/usynergy -I ./src/libs/ogl -I ./src/libs/glm -DIMGUI_IMPL_OPENGL_LOADER_GL3W
	CFLAGS = $(CXXFLAGS)
endif

g: preconditions $(EXE)

preconditions:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/imGui
	mkdir -p $(BUILD_DIR)/libs/ogl
	mkdir -p $(BUILD_DIR)/libs/usynergy
	mkdir -p $(BUILD_DIR)/libs/gl3w/GL
	mkdir -p $(BUILD_DIR)/libs/glm/detail
	mkdir -p bin

$(BUILD_DIR)/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(EXE): $(OBJ_FILES)
	$(CXX) -o bin/$@ $^ $(CXXFLAGS) $(LIBS)
	@echo $(BUILD_DIR) complete for $(ECHO_MESSAGE)
	bin/$(EXE)

clean:
	rm -f bin/$(EXE) $(BUILD_DIR)/*.o $(BUILD_DIR)/imGui/*.o $(BUILD_DIR)/libs/ogl/*.o $(BUILD_DIR)/libs/gl3w/GL/*.o $(BUILD_DIR)/libs/glm/detail/*.o

clean-win:
	rm -f bin/$(EXE).exe $(BUILD_DIR)/*.o $(BUILD_DIR)/imGui/*.o $(BUILD_DIR)/libs/ogl/*.o $(BUILD_DIR)/libs/gl3w/GL/*.o $(BUILD_DIR)/libs/glm/detail/*.o