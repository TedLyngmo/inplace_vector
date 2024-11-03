#include "inplace_vector.hpp"

#include <iostream>
#include <string>

int main() {
    cpp26::inplace_vector<std::string, 4> iv;

    iv.emplace_back("1. Hello, this is a pretty long string that will not fit in SSO");
    iv.emplace_back("2. world, now this is very funny stuff to fix and trix with");
    iv.emplace_back("3. whohoo, this is also a long string with no SSO I hope");
    iv.emplace_back("4. yet another long string that will be moved");
    for(auto& str : iv) std::cout << str << '\n';
    std::cout << "---\n";
    auto it = iv.erase(iv.cbegin() + 1, iv.cbegin() + 3);
    std::cout << *it << '\n';
    std::cout << "---\n";
    for(auto& str : iv) std::cout << str << '\n';
}
