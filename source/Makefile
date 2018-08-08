all: tinyply-core

tinyply-core: tinyply.h tinyply.cpp example.cpp
	$(CXX) tinyply.cpp example.cpp -std=c++11 -o $@ -Wall -Wpedantic

.PHONY: clean
clean:
	rm tinyply-core
	