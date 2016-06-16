MAKEFLAGS = "-j 10"
DEBUG = -g -DTIMING -DONLINE
CC = g++

THRIFT_DIR = gen-cpp

LIBS = -lthrift -lglog
INCS = 
CFLAGS = $(DEBUG) $(INCS) -Wall -Wno-reorder -std=c++11

OBJ_DIR = .build
BIN_DIR = bin

EXCLUDE_SKELETON = $(wildcard $(THRIFT_DIR)/*.skeleton.cpp)
CPPS = $(filter-out $(EXCLUDE_SKELETON), $(wildcard gen-cpp/*.cpp *.cpp))
OBJS = $(CPPS:%.cpp=$(OBJ_DIR)/%.o)
DEPS = $(OBJS:%.o=%.d)

OBJS_WO_CLIENT = $(filter-out $(OBJ_DIR)/client.o, $(OBJS))
OBJS_WO_SERVER = $(filter-out $(OBJ_DIR)/server.o, $(OBJS))

TEST_CPPS = $(wildcard test/*/*_test.cpp test/*/*/*_test.cpp)
TEST_OBJS = $(TEST_CPPS:%.cpp=$(OBJ_DIR)/%.o)
TEST_DEPS = $(TEST_OBJS:%.o=%.d)
TEST_BINS = $(TEST_CPPS:%.cpp=$(BIN_DIR)/%)

all: $(BIN_DIR)/client $(BIN_DIR)/server

-include $(DEPS) $(TEST_DEPS)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MM -MT $@ -MF $(patsubst %.o,%.d,$@) $<
	$(CC) $(CFLAGS) -c -o $@ $<

$(BIN_DIR)/client: $(OBJS_WO_SERVER) $(OBJ_DIR)/client.o
	@mkdir -p $(@D)
	$(CC) -o $@ $^ $(LIBS)

$(BIN_DIR)/server: $(OBJS_WO_CLIENT) $(OBJ_DIR)/server.o
	@mkdir -p $(@D)
	$(CC) -o $@ $^ $(LIBS) -lgtest

.PHONY: clean
clean:
	rm -rf $(BIN_DIR)/engine $(TEST_BINS) $(OBJ_DIR)

