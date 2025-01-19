#include "inplace_vector.hpp"

#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

// empty:
template class cpp26::inplace_vector<int, 0>;

// trivial non-empty:
template class cpp26::inplace_vector<int, 1>;
template class cpp26::inplace_vector<int, 2>;
template class cpp26::inplace_vector<const int, 3>;

// non-trivial
template class cpp26::inplace_vector<std::string, 3>;
template class cpp26::inplace_vector<const std::string, 3>;

// move-only:
template class cpp26::inplace_vector<const std::unique_ptr<int>, 3>;
template class cpp26::inplace_vector<std::unique_ptr<int>, 3>;

template<class...>
struct is_inplace_vector : std::false_type {};
template<class T, std::size_t N>
struct is_inplace_vector<cpp26::inplace_vector<T, N>> : std::true_type {};

static_assert(not is_inplace_vector<int>::value, "");
static_assert(is_inplace_vector<cpp26::inplace_vector<int, 0>>::value, "");

namespace {
template<class T>
auto str(T&& val) -> typename std::enable_if<not is_inplace_vector<typename std::remove_reference<T>::type>::value, T>::type {
    return val;
}

template<class T, std::size_t N>
std::string str(const cpp26::inplace_vector<T, N>& vec) {
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

template<class L, class R, class Cond>
bool Assert(L&& lhs, R&& rhs, int line, Cond cond, const char* condstr) {
    if(!cond(lhs, rhs)) {
        std::cout << "FAIL @ line " << line << ": " << str(lhs) << ' ' << condstr << ' ' << str(rhs) << '\n';
        return true;
    }
    return false;
}
#define ASSERT_EQ(lhs, rhs)                                                                                \
    do {                                                                                                   \
        Fail = Assert(                                                                                     \
                   lhs, rhs, __LINE__,                                                                     \
                   [](typename std::remove_reference<decltype(lhs)>::type const& l,                        \
                      typename std::remove_reference<decltype(rhs)>::type const& r) { return l == r; },    \
                   "NOT !=") ||                                                                            \
               Fail;                                                                                       \
        Fail = Assert(                                                                                     \
                   lhs, rhs, __LINE__,                                                                     \
                   [](typename std::remove_reference<decltype(lhs)>::type const& l,                        \
                      typename std::remove_reference<decltype(rhs)>::type const& r) { return !(l != r); }, \
                   "NOT !=") ||                                                                            \
               Fail;                                                                                       \
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

#if __cplusplus >= 201402L
namespace detail {
template<class T, std::size_t... Is>
constexpr cpp26::inplace_vector<T, sizeof...(Is)> make_inplace_vector_helper(std::index_sequence<Is...>) {
    return cpp26::inplace_vector<T, sizeof...(Is)>{static_cast<T>(Is)...};
}
} // namespace detail
template<class T, size_t N>
constexpr cpp26::inplace_vector<T, N> make_inplace_vector() {
    static_assert(!std::is_trivially_copyable<T>::value || std::is_trivially_copyable<cpp26::inplace_vector<T, N>>::value || N == 0,
                  "triviality failure");
    return detail::make_inplace_vector_helper<T>(std::make_index_sequence<N>{});
}

template<class T, size_t N>
constexpr bool constexpr_test() {
    bool fail = false;
    for(unsigned i = 0; i < 10; ++i) {
        const auto cxpr = make_inplace_vector<T, N>();
        fail = (cxpr.size() != N) || fail;
        if(N != 0) {
            fail = (cxpr.front() != 0) || fail;
            fail = (cxpr[0] != 0) || fail;
            fail = (cxpr.back() != N - 1) || fail;
            fail = (cxpr[N - 1] != N - 1) || fail;
        } else {
            fail = (sizeof cxpr != 1) || fail;
        }

        using st = typename decltype(cxpr)::size_type;
        const auto s = cxpr.size();
        for(st j = 0; j < s; ++j) {
            fail = (cxpr[j] != j) || fail;
        }
    }
    return not fail;
}
#endif

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
                  {"2.3 - I am the second string", "2.7 - and I am the third string, the fourth one is lying"});
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
    std::cout << "--- move only\n";
    {
        std::vector<std::unique_ptr<int>> vu;
        vu.emplace_back(new int{1});
        vu.emplace_back(new int{2});
        vu.emplace_back(new int{3});
        cpp26::inplace_vector<std::unique_ptr<int>, 3> mov;
        mov.assign(std::make_move_iterator(vu.begin()), std::make_move_iterator(vu.end()));
        assert(*mov[0] == 1);
        assert(*mov[1] == 2);
        assert(*mov[2] == 3);

        auto empty_other = std::move(mov);
        ASSERT_EQ(mov.size(), size_t{0});
        ASSERT_EQ(mov.empty(), true);
        assert(*empty_other[0] == 1);
        assert(*empty_other[1] == 2);
        assert(*empty_other[2] == 3);
    }
    std::cout << "--- trivial\n";
    {
        cpp26::inplace_vector<int, 4> foo{1, 2, 3, 4};
        auto bar = std::move(foo);
        ASSERT_EQ(foo.size(), size_t{4});
        ASSERT_EQ(foo[0], 1);
        ASSERT_EQ(foo[1], 2);
        ASSERT_EQ(foo[2], 3);
        ASSERT_EQ(foo[3], 4);
        ASSERT_EQ(bar.size(), size_t{4});
        ASSERT_EQ(bar[0], 1);
        ASSERT_EQ(bar[1], 2);
        ASSERT_EQ(bar[2], 3);
        ASSERT_EQ(bar[3], 4);
        cpp26::inplace_vector<int, 4> baz;
        baz = bar;
        ASSERT_EQ(baz.size(), size_t{4});
        ASSERT_EQ(baz[0], 1);
        ASSERT_EQ(baz[1], 2);
        ASSERT_EQ(baz[2], 3);
        ASSERT_EQ(baz[3], 4);
    }

#if __cplusplus >= 202002L
    std::cout << "--- constexpr\n";
    {
        static_assert(constexpr_test<unsigned, 0>());
        static_assert(constexpr_test<unsigned, 1>());
        static_assert(constexpr_test<unsigned, 2>());
        static_assert(constexpr_test<unsigned, 3>());
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
