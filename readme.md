# tinyply

A two-file, zero-dependency (except the C++ STL) public domain implementation of the PLY mesh file format. An overview and definition of the file format is available [here](http://paulbourke.net/dataformats/ply/). This format is often used in the computer vision community for its relative simplicity and ability to support arbitrary mesh attributes and layouts.

The library is written in C++11 and requires a recent compiler (GCC 4.8+ / VS2013+ / Clang 2.9+). Tinyply supports exporting and importing PLY files in both binary and ascii formats. Recently, `tinyply` was modified to support filesizes >= 4gb and read big-endian binary formats. The library does not directly perform file i/o for either reading or writing.

## Getting Started

The project comes with a simple example program demonstrating a circular write / read and all of the major API functionality. 

## License

This software is in the public domain. Where that dedication is not recognized, you are granted a perpetual, irrevocable license to copy, distribute, and modify this file as you see fit. If these terms are not suitable to your organization, you may choose to license it under the terms of the 2-clause simplified BSD. 
