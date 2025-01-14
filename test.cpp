
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "inplace_vector.hpp"
int main() {
    cpp26::inplace_vector<std::string, 4> iv;
    cpp26::inplace_vector<std::string, 4> other;

    std::cout << "--- emplace 4 strings\n";
    {
        iv.emplace_back("1. Hello, this is a pretty long string that will not fit in SSO");
        iv.emplace_back("2. world, now this is very funny stuff to fix and trix with");
        iv.emplace_back("3. whohoo, this is also a long string with no SSO I hope");
        iv.emplace_back("4. yet another long string that will be moved");
        assert(iv.size() == 4);
        for(auto& str : iv) std::cout << str << '\n';
    }
    std::cout << "--- Reverse iterator\n";
    {
        for(auto it = iv.crbegin(); it != iv.crend(); ++it) std::cout << *it << "\n";
    }
    std::cout << "--- erase two in the middle\n";
    {
        auto it = iv.erase(iv.cbegin() + 1, iv.cbegin() + 3);
        assert(iv.size() == 2);
        std::cout << *it << '\n';
        std::swap(iv, other);
        assert(iv.size() == 0);
        assert(other.size() == 2);
        for(auto& str : other) std::cout << str << '\n';
    }
    std::cout << "--- with one added at end:\n";
    {
        other.insert(other.end(), "5. a long string added at end will be nice");
        assert(other.size() == 3);
        for(auto& str : other) std::cout << str << '\n';
    }
    std::cout << "--- with one added at begin:\n";
    {
        other.insert(other.begin(), "0. a long string added at begin is fine");
        assert(other.size() == 4);
        for(auto& str : other) std::cout << str << '\n';
    }
    std::cout << "--- insert with count\n";
    {
        iv.clear();
        assert(iv.empty());
        iv.insert(iv.begin(), 3, "Here be three copies of the same string inserted");
        auto& teststr = "And one inserted second";
        auto it = iv.insert(iv.begin() + 1, 1, teststr);
        assert(*it == teststr);
        for(auto& str : iv) std::cout << str << '\n';
    }
    std::cout << "--- insert with iterators\n";
    {
        iv.resize(2);
        assert(iv.size() == 2);
        std::string two[]{"2. Now \"Here be three copies...\" is first and \"And one inserted second\" last",
                          "3. And I am the third string"};
        auto it = iv.insert(iv.begin() + 1, std::begin(two), std::end(two));
        for(auto& str : iv) std::cout << str << '\n';
        assert(iv.size() == 4);
        assert(*it == two[0]);
        assert(*std::next(it) == two[1]);
    }
    std::cout << "--- insert with initializer_list\n";
    {
        iv.erase(std::prev(iv.end()));
        iv.erase(iv.begin());
        assert(iv.size() == 2);
        iv.insert(iv.begin() + 1, {"2.3 - I am the second string", "2.7 - and I amd the third string, the fourth one is lying"});
        assert(iv.size() == 4);
        for(auto& str : iv) std::cout << str << '\n';
    }
}
