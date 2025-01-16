ifeq (g++,$(CXX))
OPTS=-Wall -Wextra -Wnrvo -fdiagnostics-all-candidates
else ifeq (clang++,$(CXX))
OPTS=-Weverything -fsafe-buffer-usage-suggestions -Wno-unsafe-buffer-usage -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-c++20-compat -Wno-missing-prototypes #-Wno-ctad-maybe-unsupported
else
OPTS=-Wall -Wextra
endif

.PHONY:
test: cpp11 cpp14 cpp17 cpp20 cpp23 cpp26
	@echo OK $(CXX) $(OPTS)

cpp26: test.cpp include/inplace_vector.hpp Makefile
	$(CXX) -std=c++26 -o $@ $< -Iinclude $(OPTS) -Werror -pedantic -g -fsanitize=address,undefined && ./$@

cpp23: test.cpp include/inplace_vector.hpp Makefile
	$(CXX) -std=c++23 -o $@ $< -Iinclude $(OPTS) -Werror -pedantic -g -fsanitize=address,undefined && ./$@

cpp20: test.cpp include/inplace_vector.hpp Makefile
	$(CXX) -std=c++20 -o $@ $< -Iinclude $(OPTS) -Werror -pedantic -g -fsanitize=address,undefined && ./$@

cpp17: test.cpp include/inplace_vector.hpp Makefile
	$(CXX) -std=c++17 -o $@ $< -Iinclude $(OPTS) -Werror -pedantic -g -fsanitize=address,undefined && ./$@

cpp14: test.cpp include/inplace_vector.hpp Makefile
	$(CXX) -std=c++14 -o $@ $< -Iinclude $(OPTS) -Werror -pedantic -g -fsanitize=address,undefined && ./$@

cpp11: test.cpp include/inplace_vector.hpp Makefile
	$(CXX) -std=c++11 -o $@ $< -Iinclude $(OPTS) -Werror -pedantic -g -fsanitize=address,undefined && ./$@

cpp98: test.cpp include/inplace_vector.hpp Makefile
	$(CXX) -std=c++98 -o $@ $< -Iinclude $(OPTS) -Werror -pedantic -g -fsanitize=address,undefined && ./$@

clean:
	rm -f cpp11 cpp14 cpp17 cpp20 cpp23 cpp26
