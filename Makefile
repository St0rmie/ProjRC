CC = g++
CXX = g++
LD = g++

INCLUDE_DIRS := src/client src/server src/
INCLUDES = $(addprefix -I, $(INCLUDE_DIRS))

TARGETS = src/client/client src/server/server
TARGET_EXECS = client

CLIENT_SOURCES := $(wildcard src/client/*.cpp)
SHARED_SOURCES := $(wildcard src/shared/*.cpp)
SERVER_SOURCES := $(wildcard src/server/*.cpp)
SOURCES := $(CLIENT_SOURCES) $(SHARED_SOURCES) $(SERVER_SOURCES)

CLIENT_HEADERS := $(wildcard src/client/*.hpp)
SHARED_HEADERS := $(wildcard src/shared/*.hpp)
SERVER_HEADERS := $(wildcard src/server/*.hpp)
HEADERS := $(CLIENT_HEADERS) $(SHARED_HEADERS) $(SERVER_HEADERS)

CLIENT_OBJECTS := $(CLIENT_SOURCES:.cpp=.o)
SHARED_OBJECTS := $(SHARED_SOURCES:.cpp=.o)
SERVER_OBJECTS := $(SERVER_SOURCES:.cpp=.o)
OBJECTS := $(CLIENT_OBJECTS) $(SHARED_OBJECTS) $(SERVER_OBJECTS)

CXXFLAGS = -std=c++17
LDFLAGS = -std=c++17

CXXFLAGS += $(INCLUDES)
LDFLAGS += $(INCLUDES)

#CXXFLAGS += -fdiagnostics-color=always
#CXXFLAGS += -Wall
#CXXFLAGS += -Werror
#CXXFLAGS += -Wextra
#CXXFLAGS += -Wcast-align
#CXXFLAGS += -Wconversion
#CXXFLAGS += -Wfloat-equal
#CXXFLAGS += -Wformat=2
#CXXFLAGS += -Wnull-dereference
#CXXFLAGS += -Wshadow
#CXXFLAGS += -Wsign-conversion
#CXXFLAGS += -Wswitch-default
#CXXFLAGS += -Wswitch-enum
#CXXFLAGS += -Wundef
#CXXFLAGS += -Wunreachable-code
#CXXFLAGS += -Wunused
LDFLAGS += -pthread

all: $(TARGET_EXECS)

fmt: $(SOURCES) $(HEADERS)
	clang-format -i $^

fmt-check: $(SOURCES) $(HEADERS)
	clang-format -n --Werror $^

src/server/server: $(SERVER_OBJECTS) $(SERVER_HEADERS) $(SHARED_OBJECTS) $(SHARED_HEADERS)
src/client/client: $(CLIENT_OBJECTS) $(CLIENT_HEADERS) $(SHARED_OBJECTS) $(SHARED_HEADERS)

server: src/server/server
	cp src/server/server server

client: 
	$(CXX) -o client $(CXXFLAGS) $(CLIENT_SOURCES) $(SHARED_SOURCES) $(CLIENT_HEADERS) $(SHARED_HEADERS)

clean:
	rm -f $(OBJECTS) $(TARGETS) $(TARGET_EXECS) project.zip

package:
	zip project.zip $(SOURCES) $(HEADERS) Makefile .clang-format readme.txt
