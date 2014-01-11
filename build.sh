#!/bin/bash

g++ -O3 -Wall  -c paser_swf.cpp
g++ -o paser_swf paser_swf.o /usr/lib/libz.a --static
