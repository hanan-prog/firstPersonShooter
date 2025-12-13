CXX := clang++
CC  := clang

CXXFLAGS := -I. -F/Library/Frameworks -fsanitize=address -std=c++11
CFLAGS   := -I. -F/Library/Frameworks -fsanitize=address

OPTFLAGS := -O2

TARGET := shooter

# C++ sources
SRCS_CPP := main.cpp
SRCS_CC  := shader.cc 

# C sources
SRCS_C   := glad/glad.c

# Object files (ONLY .o here)
OBJS := $(SRCS_CPP:.cpp=.o) \
        $(SRCS_CC:.cc=.o)  \
        $(SRCS_C:.c=.o)

LDFLAGS := -fsanitize=address \
           -F/Library/Frameworks \
           -framework SDL3 \
           -Wl,-rpath,/Library/Frameworks

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile rules
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
