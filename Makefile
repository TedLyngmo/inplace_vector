.PHONY:
test: cpp11 cpp14 cpp17 cpp20 cpp23 cpp26
	@echo OK

cpp26: test.cpp include/inplace_vector.hpp Makefile
	$(CXX) -std=c++26 -o $@ $< -Iinclude -Wall -Wextra -pedantic-errors -g -fsanitize=address,undefined && ./$@

cpp23: test.cpp include/inplace_vector.hpp Makefile
	$(CXX) -std=c++23 -o $@ $< -Iinclude -Wall -Wextra -pedantic-errors -g -fsanitize=address,undefined && ./$@

cpp20: test.cpp include/inplace_vector.hpp Makefile
	$(CXX) -std=c++20 -o $@ $< -Iinclude -Wall -Wextra -pedantic-errors -g -fsanitize=address,undefined && ./$@

cpp17: test.cpp include/inplace_vector.hpp Makefile
	$(CXX) -std=c++17 -o $@ $< -Iinclude -Wall -Wextra -pedantic-errors -g -fsanitize=address,undefined && ./$@

cpp14: test.cpp include/inplace_vector.hpp Makefile
	$(CXX) -std=c++14 -o $@ $< -Iinclude -Wall -Wextra -pedantic-errors -g -fsanitize=address,undefined && ./$@

cpp11: test.cpp include/inplace_vector.hpp Makefile
	$(CXX) -std=c++11 -o $@ $< -Iinclude -Wall -Wextra -pedantic-errors -g -fsanitize=address,undefined && ./$@

cpp98: test.cpp include/inplace_vector.hpp Makefile
	$(CXX) -std=c++98 -o $@ $< -Iinclude -Wall -Wextra -pedantic-errors -g -fsanitize=address,undefined && ./$@
