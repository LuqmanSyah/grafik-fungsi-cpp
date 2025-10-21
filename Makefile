# Simple Makefile for building the SFML graph program

CXX = g++
SRC = main.cpp
OUT = main

# Try pkg-config first; if not available, fall back to explicit link flags
PKG_CFLAGS := $(shell pkg-config --cflags sfml-all 2>/dev/null)
PKG_LIBS   := $(shell pkg-config --libs sfml-all 2>/dev/null)

ifeq ($(PKG_LIBS),)
    LIBS = -lsfml-graphics -lsfml-window -lsfml-system
    CFLAGS =
else
    LIBS = $(PKG_LIBS)
    CFLAGS = $(PKG_CFLAGS)
endif

all: $(OUT)

$(OUT): $(SRC)
	$(CXX) $(CFLAGS) $(SRC) -o $(OUT) $(LIBS)

run: all
	./$(OUT)

clean:
	rm -f $(OUT)
