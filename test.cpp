#include "inplace_vector.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>

namespace {
template<class T>
std::string str(T&& vec) {
    std::ostringstream os;
    os << '{';
    if(not vec.empty()) {
        auto it = std::begin(vec);
        os << *it;
        std::advance(it, 1);
        for(auto end = std::end(vec); it != end; std::advance(it, 1)) {
            os << ',' << *it;
        }
    }
    os << '}';
    return os.str();
}
bool Fail = false;
} // namespace

template<class T, class Cond>
bool Assert(T&& lhs, T&& rhs, int line, Cond cond, const char* condstr) {
    if(!cond(lhs, rhs)) {
        std::cout << "FAIL @ line " << line << ": " << str(lhs) << ' ' << condstr << ' ' << str(rhs) << '\n';
        return true;
    }
    return false;
}
#define ASSERT_EQ(lhs, rhs)                                                                                                \
    do {                                                                                                                   \
        Fail = Assert(lhs, rhs, __LINE__, std::equal_to<decltype(lhs)>{}, "==") || Fail;                                   \
        Fail = Assert(lhs, rhs, __LINE__, [](decltype(lhs)& l, decltype(rhs)& r) { return !(l != r); }, "NOT !=") || Fail; \
    } while(false)

#define ASSERT_NOT_EQ(lhs, rhs)                                                                                            \
    do {                                                                                                                   \
        Fail = Assert(lhs, rhs, __LINE__, [](decltype(lhs)& l, decltype(rhs)& r) { return l != r; }, "!=") || Fail;        \
        Fail = Assert(lhs, rhs, __LINE__, [](decltype(lhs)& l, decltype(rhs)& r) { return !(l == r); }, "NOT ==") || Fail; \
    } while(false)

#define ASSERT_LT(lhs, rhs)                                                         \
    do {                                                                            \
        Fail = Assert(lhs, rhs, __LINE__, std::less<decltype(lhs)>{}, "<") || Fail; \
    } while(false)
#define ASSERT_NOT_LT(lhs, rhs)                                                                                          \
    do {                                                                                                                 \
        Fail = Assert(lhs, rhs, __LINE__, [](decltype(lhs)& l, decltype(rhs)& r) { return !(l < r); }, "NOT <") || Fail; \
    } while(false)

#define ASSERT_GT(lhs, rhs)                                                            \
    do {                                                                               \
        Fail = Assert(lhs, rhs, __LINE__, std::greater<decltype(lhs)>{}, ">") || Fail; \
    } while(false)
#define ASSERT_NOT_GT(lhs, rhs)                                                                                          \
    do {                                                                                                                 \
        Fail = Assert(lhs, rhs, __LINE__, [](decltype(lhs)& l, decltype(rhs)& r) { return !(l > r); }, "NOT >") || Fail; \
    } while(false)

#define ASSERT_LTEQ(lhs, rhs)                                                              \
    do {                                                                                   \
        Fail = Assert(lhs, rhs, __LINE__, std::less_equal<decltype(lhs)>{}, "<=") || Fail; \
    } while(false)
#define ASSERT_NOT_LTEQ(lhs, rhs)                                                                                          \
    do {                                                                                                                   \
        Fail = Assert(lhs, rhs, __LINE__, [](decltype(lhs)& l, decltype(rhs)& r) { return !(l <= r); }, "NOT <=") || Fail; \
    } while(false)

#define ASSERT_GTEQ(lhs, rhs)                                                                 \
    do {                                                                                      \
        Fail = Assert(lhs, rhs, __LINE__, std::greater_equal<decltype(lhs)>{}, ">=") || Fail; \
    } while(false)
#define ASSERT_NOT_GTEQ(lhs, rhs)                                                                                          \
    do {                                                                                                                   \
        Fail = Assert(lhs, rhs, __LINE__, [](decltype(lhs)& l, decltype(rhs)& r) { return !(l >= r); }, "NOT >=") || Fail; \
    } while(false)

int main() {
    using T = std::string;
    using IVS = cpp26::inplace_vector<T, 4>;
    IVS iv;
    IVS other;
    // validate General container requirements
    static_assert(std::is_same<T, typename IVS::value_type>::value, "");
    static_assert(std::is_same<T&, typename IVS::reference>::value, "");
    static_assert(std::is_same<T const&, typename IVS::const_reference>::value, "");
    static_assert(std::is_integral<typename IVS::difference_type>::value, "");
    static_assert(std::is_signed<typename IVS::difference_type>::value, "");
    static_assert(std::is_integral<typename IVS::size_type>::value, "");
    static_assert(std::is_unsigned<typename IVS::size_type>::value, "");

    // not a requirement, but validates that the iterators are there:
    static_assert(std::is_same<T*, typename IVS::iterator>::value, "");
    static_assert(std::is_same<T const*, typename IVS::const_iterator>::value, "");

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
        auto it = iv.erase(std::next(iv.cbegin()), std::next(iv.cbegin(), 3));
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
        auto it = iv.insert(std::next(iv.begin()), 1, teststr);
        assert(*it == teststr);
        for(auto& str : iv) std::cout << str << '\n';
    }
    std::cout << "--- insert with iterators\n";
    {
        iv.resize(2);
        assert(iv.size() == 2);
        std::string two[]{"2. Now \"Here be three copies...\" is first and \"And one inserted second\" last",
                          "3. And I am the third string"};
        auto it = iv.insert(std::next(iv.begin()), std::begin(two), std::end(two));
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
        iv.insert(std::next(iv.begin()),
                  {"2.3 - I am the second string", "2.7 - and I amd the third string, the fourth one is lying"});
        assert(iv.size() == 4);
        for(auto& str : iv) std::cout << str << '\n';
    }
    {
        bool ex = false;
        try {
            iv.emplace(iv.begin());
        } catch(const std::bad_alloc&) {
            ex = true;
        }
        assert(ex == true);
        assert(iv.try_push_back("nope") == nullptr);
        assert(iv.try_emplace_back("nope") == nullptr);
    }
    std::cout << "--- comparisons\n";
    {
        iv.clear();
        other.clear();

        ASSERT_EQ(iv, other);
        ASSERT_NOT_LT(iv, other);
        ASSERT_NOT_GT(iv, other);
        ASSERT_LTEQ(iv, other);
        ASSERT_GTEQ(iv, other);

        other.emplace_back("1");
        ASSERT_NOT_EQ(iv, other);
        ASSERT_LT(iv, other);
        ASSERT_NOT_GT(iv, other);
        ASSERT_LTEQ(iv, other);
        ASSERT_NOT_GTEQ(iv, other);

        iv.emplace_back("2");
        ASSERT_NOT_EQ(iv, other);
        ASSERT_NOT_LT(iv, other);
        ASSERT_GT(iv, other);
        ASSERT_NOT_LTEQ(iv, other);
        ASSERT_GTEQ(iv, other);

        iv.pop_back();
        iv.emplace_back("1");
        ASSERT_EQ(iv, other);
        ASSERT_NOT_LT(iv, other);
        ASSERT_NOT_GT(iv, other);
        ASSERT_LTEQ(iv, other);
        ASSERT_GTEQ(iv, other);

        other.emplace_back("2");
        ASSERT_NOT_EQ(iv, other);
        ASSERT_LT(iv, other);
        ASSERT_NOT_GT(iv, other);
        ASSERT_LTEQ(iv, other);
        ASSERT_NOT_GTEQ(iv, other);
    }
    {
        // const access
        [](const cpp26::inplace_vector<std::string, 4>& r) {
            for(auto& _ : r) {
                (void)_;
            }
        }(iv);
    }
    {
        std::vector<std::unique_ptr<int>> vu;
        vu.emplace_back(new int);
        vu.emplace_back(new int);
        vu.emplace_back(new int);
        cpp26::inplace_vector<std::unique_ptr<int>, 3> mov;
        mov.assign(std::make_move_iterator(vu.begin()), std::make_move_iterator(vu.end()));
    }

#if __cplusplus >= 202002L
    {
        /*
        constexpr cpp26::inplace_vector<int, 4> cxpr(1);
        static_assert(cxpr.size() == 1);
        static_assert(cxpr.back() == 0);
        static_assert(cxpr.front() == 0);
        static_assert(cxpr[0] == 0);
        */
    }
    std::cout << "--- assign_range\n";
    {
        std::vector<std::string> v1{"Hello", "world"};
        cpp26::inplace_vector<std::string, 3> r1{"1", "2", "3"};
        r1.assign_range(v1);
        assert(std::equal(v1.begin(), v1.end(), r1.begin(), r1.end()));
    }
    std::cout << "--- append_range\n";
    {
        std::vector<std::string> v1{"Hello", "world"};
        cpp26::inplace_vector<std::string, 3> r1;
        r1.append_range(v1);
        assert(std::equal(v1.begin(), v1.end(), r1.begin(), r1.end()));
    }
    std::cout << "--- try_append_range\n";
    {
        std::vector<std::string> v1{"Hello", "world", "now", "we", "will", "see"};
        cpp26::inplace_vector<std::string, 2> r1;
        auto it = r1.try_append_range(v1);
        assert(*it == "now");
        assert(std::equal(v1.begin(), std::next(v1.begin(), 2), r1.begin(), r1.end()));
    }
#endif

#if __cplusplus >= 202302L && defined(__cpp_lib_containers_ranges)
    std::cout << "--- from_range\n";
    {
        std::vector<std::string> v1{"Hello", "world"};
        cpp26::inplace_vector<std::string, 4> r1(std::from_range, v1);
        assert(std::equal(v1.begin(), v1.end(), r1.begin(), r1.end()));
    }
#endif
    return Fail;
}
