
BUILD_DIR             =build
CPP                   =g++
INCLUDE_V8            =archives/v8-read-only/include
BINLIB_V8             =archives/v8-read-only/libv8.a
LIBS                  =-lpthread
INCLUDES              =-I$(INCLUDE_V8)
PRODUCT               =k7


all:
	g++ $(INCLUDES) src/main.cpp -o $(PRODUCT) $(BINLIB_V8) $(LIBS)

