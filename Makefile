CC = g++
CXX = g++
LD = g++

INCLUDE_DIRS := src/client src/server src/
INCLUDES = $(addprefix -I, $(INCLUDE_DIRS))

TARGETS = src/client/user src/server/server
TARGET_EXECS = user AS

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

vpath # clears VPATH
vpath %.hpp $(INCLUDE_DIRS)

# Run `make OPTIM=no` to disable -O3
ifeq ($(strip $(OPTIM)), no)
	CXXFLAGS += -O0
else
	CXXFLAGS += -O3
endif

# Run `make DEBUG=true` to run with debug symbols
ifeq ($(strip $(DEBUG)), yes)
	CXXFLAGS += -g
endif

LDFLAGS = -fsanitize=address -lasan

#CXXFLAGS += -fdiagnostics-color=always
#CXXFLAGS += -Wall					Comparison issues
CXXFLAGS += -Werror
#CXXFLAGS += -Wextra 				Missing comparisons and unused minor stuff
CXXFLAGS += -Wcast-align
#CXXFLAGS += -Wconversion 			HELL
CXXFLAGS += -Wfloat-equal
CXXFLAGS += -Wformat=2
CXXFLAGS += -Wnull-dereference
CXXFLAGS += -Wshadow
#CXXFLAGS += -Wsign-conversion  	HELL
CXXFLAGS += -Wswitch-default
CXXFLAGS += -Wswitch-enum
CXXFLAGS += -Wundef
CXXFLAGS += -Wunreachable-code
CXXFLAGS += -Wunused
LDFLAGS += -pthread


.PHONY: all clean fmt fmt-check package

all: $(TARGET_EXECS)

fmt: $(SOURCES) $(HEADERS)
	clang-format -i $^

fmt-check: $(SOURCES) $(HEADERS)
	clang-format -n --Werror $^

AS: $(SERVER_OBJECTS) $(SERVER_HEADERS) $(SHARED_OBJECTS) $(SHARED_HEADERS)
	$(CC) -o AS $(LDFLAGS) $(CXXFLAGS) $(SERVER_OBJECTS) $(SERVER_HEADERS) $(SHARED_OBJECTS) $(SHARED_HEADERS) 
user: $(CLIENT_OBJECTS) $(CLIENT_HEADERS) $(SHARED_OBJECTS) $(SHARED_HEADERS)
	$(CC) -o user $(LDFLAGS) $(CXXFLAGS) $(CLIENT_OBJECTS) $(CLIENT_HEADERS) $(SHARED_OBJECTS) $(SHARED_HEADERS) 

clean:
	rm -f $(OBJECTS) $(TARGETS) $(TARGET_EXECS) project.zip *.html

clean-database:
	rm -rf ASDIR

package:
	cp README.md readme.txt
	zip project.zip src $(SOURCES) $(HEADERS) Makefile .clang-format readme.txt tester
	rm readme.txt
