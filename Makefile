all: tinyply-core

tinyply-core: ./source/tinyply.cpp
	$(CXX) tinyply.cpp -std=c++11 -o $@