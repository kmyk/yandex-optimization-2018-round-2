.PHONY: build

CXX = g++
CXXFLAGS = -std=c++14 -Wall -O2 -g -DLOCAL

build: a.out
a.out: main.cpp
	${CXX} ${CXXFLAGS} $<

testgen: testgen.cpp testlib.h
	${CXX} ${CXXFLAGS} $< -o $@
checker: checker.cpp testlib.h
	${CXX} ${CXXFLAGS} $< -o $@
