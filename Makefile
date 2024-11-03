test: test.cpp include/inplace_vector.hpp Makefile
	$(CXX) -std=c++17 -o $@ $< -Iinclude -Wall -Wextra -pedantic-errors -g -fsanitize=address,undefined
