#definitions
DIR_SRC = ./
DIR_OBJ = ./obj
SOURCE  := $(wildcard ${DIR_SRC}/*.c) $(wildcard ${DIR_SRC}/*.cpp)
OBJS    := $(patsubst ${DIR_SRC}/%.c,${DIR_OBJ}/%.o,$(patsubst ${DIR_SRC}/%.cpp,${DIR_OBJ}/%.o,$(SOURCE)))
TARGET  := main

#compiling parameters
CC      := g++
LIBS    :=
LDFLAGS :=
DEFINES := $(FLAG)
INCLUDE :=
CFLAGS  := -g -Wall -O3 -std=c++11 $(DEFINES) $(INCLUDE)
CXXFLAGS:= $(CFLAGS)

#commands
.PHONY : all objs rebuild clean init ctag
all : ctag init $(TARGET)

objs : init $(OBJS)

rebuild : clean all

clean :
	rm -rf $(DIR_OBJ)
	rm -f $(TARGET)
	rm -f tags

init :
	if [ ! -d obj ]; then mkdir obj; fi

ctag :
	ctags -R

$(TARGET) : $(OBJS)
	$(CC) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)

${DIR_OBJ}/%.o:${DIR_SRC}/%.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@
