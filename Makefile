
.PHONY: all configure build clean

all: configure build

configure:
	./waf configure

build:
	./waf build

clean:
	./waf clean
