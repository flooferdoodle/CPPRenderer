# define CPPFLAGS=-I... for other (system) includes
# define LDFLAGS=-L... for other (system) libs to link

CC = g++ -g -Wno-narrowing -Wreturn-type -Wunused-function -Wreorder -Wunused-variable -Wfloat-conversion

CC_DEBUG = @$(CC) -std=c++17
CC_RELEASE = @$(CC) -std=c++17 -O3 -DNDEBUG

G_APPS = $(wildcard apps/*)

G_DEPS = $(wildcard *.cpp *.h apps/* src/* include/*)

G_SRC = $(wildcard src/*/*.cpp src/*.cpp *.cpp)

G_INC = $(CPPFLAGS)

G_LINK = $(LDFLAGS)

all: render test

render : $(G_DEPS)
	$(CC_DEBUG) $(G_INC) $(G_SRC) apps/draw.cpp -o render

test : $(G_DEPS)
	$(CC_DEBUG) $(G_INC) $(G_SRC) apps/testing.cpp -o test

test_release : $(G_DEPS)
	$(CC_RELEASE) $(G_INC) $(G_SRC) apps/testing.cpp -o test_release


clean:
	@rm -rf image tests bench dbench draw pa?_*.png *.dSYM *.exe

