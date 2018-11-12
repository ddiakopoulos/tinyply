# tinyply 2.2

[![Release is 2.2](http://img.shields.io/badge/release-2.2-blue.svg?style=flat)](https://raw.githubusercontent.com/ddiakopoulos/tinyply/master/source/tinyply.h)
[![License is Unlicense](http://img.shields.io/badge/license-Unlicense-blue.svg?style=flat)](http://unlicense.org/)

Platform | Build Status |
-------- | ------------ |
GCC 4.9 and Clang 3.7 | [Travis CI](http://travis-ci.org): [![Build status](http://travis-ci.org/ddiakopoulos/tinyply.svg?branch=master)](https://travis-ci.org/ddiakopoulos/tinyply) |

A single-header, zero-dependency (except the C++ STL) public domain implementation of the PLY mesh file format. An overview and definition of the file format is available [here](http://paulbourke.net/dataformats/ply/). This format is often used in the computer vision and graphics communities for its relative simplicity, ability to support arbitrary mesh attributes, and binary modes.

The library is written in C++11 and requires a recent compiler (GCC 4.8+ / VS2013+ / Clang 2.9+). Tinyply supports exporting and importing PLY files in both binary and ascii formats. Tinyply supports filesizes >= 4gb and can read big-endian binary files (but not write them). 

Version 2.0 is mostly an API re-write to support later improvements towards variable length lists. One notable change is that tinyply now produces and consumes untyped byte buffers, with type information held as metadata.

Version 2.1 contained minor bugfixes and speed improvements.

Version 2.2 is a rewrite of the inner read/write loop. Compared to tinyply 2.0, this version reads and writes binary about five times faster. When a list size hint is given for reading, the performance is approximately comparable to rply. 

## Getting Started

The project comes with a simple example program demonstrating a circular write / read and all of the major API functionality. 

## License

This software is in the public domain. Where that dedication is not recognized, you are granted a perpetual, irrevocable license to copy, distribute, and modify this file as you see fit. If these terms are not suitable to your organization, you may choose to license it under the terms of the 2-clause simplified BSD. 
