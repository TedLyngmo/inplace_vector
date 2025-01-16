/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <https://unlicense.org>
*/
// Original: https://github.com/TedLyngmo/inplace_vector

// NOLINTNEXTLINE(llvm-header-guard)
#ifndef LYNIPV_F4BA9AA8_99CD_11EF_8916_90B11C0C0FF8
#define LYNIPV_F4BA9AA8_99CD_11EF_8916_90B11C0C0FF8

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <new>
#if __cplusplus >= 202002L
# include <ranges>
#endif
#include <stdexcept>
#include <type_traits>
#include <utility>

#ifndef LYNIPV_CXX20_CONSTEXPR
# if __cplusplus >= 202002L
#  define LYNIPV_CXX20_CONSTEXPR constexpr
# else
#  define LYNIPV_CXX20_CONSTEXPR
# endif
#endif

#ifndef LYNIPV_CXX14_CONSTEXPR
# if __cplusplus >= 201402L
#  define LYNIPV_CXX14_CONSTEXPR constexpr
# else
#  define LYNIPV_CXX14_CONSTEXPR
# endif
#endif

#if __cplusplus >= 202002L
# define LYNIPV_CONSTRUCT_AT(p, ...) std::construct_at(p __VA_OPT__(, ) __VA_ARGS__)
#else
# define LYNIPV_CONSTRUCT_AT(p, ...) ::new(static_cast<void*>(p)) T(__VA_ARGS__)
#endif

namespace cpp26 {
namespace detail {
#if __cplusplus >= 202002L
    template<class R, class T>
    concept container_compatiblel_range = std::ranges::input_range<R> && std::convertible_to<std::ranges::range_reference_t<R>, T>;
#endif
} // namespace detail

#if __cplusplus >= 201703L
using std::is_nothrow_swappable;
#else
template<typename U>
struct is_nothrow_swappable : std::integral_constant<bool, noexcept(swap(std::declval<U&>(), std::declval<U&>()))> {};
#endif

namespace detail {
    template<class T, std::size_t N>
    struct aligned_storage {
        using value_type = T;
        using size_type = std::size_t;
        using reference = T&;
        using const_reference = T const&;
        using pointer = T*;
        using const_pointer = T const*;

        LYNIPV_CXX14_CONSTEXPR pointer ptr(size_type idx) { return &m_data[idx].data; }
        LYNIPV_CXX14_CONSTEXPR const_pointer ptr(size_type idx) const { return &m_data[idx].data; }
        LYNIPV_CXX14_CONSTEXPR reference ref(size_type idx) { return m_data[idx].data; }
        LYNIPV_CXX14_CONSTEXPR const_reference ref(size_type idx) const { return m_data[idx].data; }

        template<class... Args>
        LYNIPV_CXX20_CONSTEXPR reference construct(size_type idx, Args&&... args) {
            return *LYNIPV_CONSTRUCT_AT(ptr(idx), std::forward<Args>(args)...);
        }
        LYNIPV_CXX14_CONSTEXPR void destroy(size_type idx) { ref(idx).~T(); }

        LYNIPV_CXX14_CONSTEXPR reference operator[](size_type idx) { return ref(idx); }
        constexpr const_reference operator[](size_type idx) const { return ref(idx); }

        constexpr size_type size() const noexcept { return m_size; }

        LYNIPV_CXX14_CONSTEXPR size_type inc() { return ++m_size; }
        LYNIPV_CXX14_CONSTEXPR size_type dec(size_type count = 1) { return m_size -= count; }

        union raw {
            LYNIPV_CXX20_CONSTEXPR ~raw() {}
            char dummy{};
            T data;
        } m_data[N];

        size_type m_size = 0;
    };

    template<class T>
    struct aligned_storage<T, 0> { // specialization for 0 elements
        using value_type = T;
        using size_type = std::size_t;
        using reference = T&;
        using const_reference = T const&;
        using pointer = T*;
        using const_pointer = T const*;

        LYNIPV_CXX14_CONSTEXPR pointer ptr(size_type) { return nullptr; }
        LYNIPV_CXX14_CONSTEXPR const_pointer ptr(size_type) const { return nullptr; }
        LYNIPV_CXX14_CONSTEXPR reference ref(size_type);
        LYNIPV_CXX14_CONSTEXPR const_reference ref(size_type) const;

        template<class... Args>
        LYNIPV_CXX20_CONSTEXPR reference construct(size_type, Args&&...);
        LYNIPV_CXX14_CONSTEXPR void destroy(size_type);

        LYNIPV_CXX14_CONSTEXPR reference operator[](size_type);
        constexpr const_reference operator[](size_type) const;

        constexpr size_type size() const noexcept { return 0; }

        LYNIPV_CXX14_CONSTEXPR size_type inc() { return 0; }
        LYNIPV_CXX14_CONSTEXPR size_type dec(size_type = 1) { return 0; }
    };
} // namespace detail

template<class T, std::size_t N>
class inplace_vector : detail::aligned_storage<T, N> {
    using base = detail::aligned_storage<T, N>;

public:
    using base::construct;
    using base::destroy;
    using base::ptr;
    using base::ref;
    using base::size;
    using base::operator[];

    using value_type = T;
    using size_type = std::size_t;
    using reference = T&;
    using const_reference = T const&;
    using pointer = T*;
    using const_pointer = T const*;
    using iterator = T*;
    using const_iterator = T const*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using difference_type = typename std::iterator_traits<iterator>::difference_type;

private:
    LYNIPV_CXX14_CONSTEXPR void shrink_to(const size_type count) noexcept {
        while(count != size()) {
            pop_back();
        }
    }

public:
    // constructors
    constexpr inplace_vector() noexcept {}

    template<bool D = std::is_default_constructible<T>::value, typename std::enable_if<D, int>::type = 0>
    LYNIPV_CXX14_CONSTEXPR explicit inplace_vector(size_type count) {
        if(count > N) throw std::bad_alloc();
        while(count != size()) unchecked_emplace_back();
    }

    template<bool C = std::is_copy_constructible<T>::value, typename std::enable_if<C, int>::type = 0>
    LYNIPV_CXX14_CONSTEXPR inplace_vector(size_type count, const T& value) {
        if(count > N) throw std::bad_alloc();
        while(count != size()) unchecked_push_back(value);
    }

    template<class InputIt, typename std::enable_if<
                                std::is_constructible<typename std::iterator_traits<InputIt>::value_type>::value, int>::type = 0>
    LYNIPV_CXX14_CONSTEXPR inplace_vector(InputIt first, InputIt last) {
        std::copy(first, last, std::back_inserter(*this));
    }

    template<class U = T, typename std::enable_if<std::is_copy_constructible<U>::value, int>::type = 0>
    LYNIPV_CXX14_CONSTEXPR inplace_vector(const inplace_vector& other) {
        for(size_type idx = 0; idx != other.size(); ++idx) {
            unchecked_push_back(other[idx]);
        }
    }

    template<class U = T, typename std::enable_if<std::is_move_constructible<U>::value, int>::type = 0>
    LYNIPV_CXX14_CONSTEXPR inplace_vector(inplace_vector&& other) noexcept(N == 0 || std::is_nothrow_move_constructible<T>::value) {
        for(size_type idx = 0; idx != other.size(); ++idx) {
            unchecked_push_back(std::move(other[idx]));
        }
    }

    template<bool C = std::is_copy_constructible<T>::value, typename std::enable_if<C, int>::type = 0>
    constexpr inplace_vector(std::initializer_list<T> init) : inplace_vector(init.begin(), init.end()) {}

#if __cplusplus >= 202302L && defined(__cpp_lib_containers_ranges)
    template<detail::container_compatiblel_range<T> R>
    constexpr inplace_vector(std::from_range_t, R&& rg) {
        if constexpr(std::ranges::sized_range<R>) {
            if(std::ranges::size(rg) > N) throw std::bad_alloc();
            for(auto&& val : rg) unchecked_emplace_back(std::forward<decltype(val)>(val));
        } else {
            for(auto&& val : rg) emplace_back(std::forward<decltype(val)>(val));
        }
    }
#endif

    // destructor
    LYNIPV_CXX20_CONSTEXPR ~inplace_vector() noexcept { clear(); }

    // assignment
    template<class U = T>
    LYNIPV_CXX14_CONSTEXPR auto operator=(const inplace_vector& other) ->
        typename std::enable_if<std::is_copy_constructible<U>::value, inplace_vector&>::type {
        assign(other.begin(), other.end());
        return *this;
    }

    template<class U = T>
    LYNIPV_CXX14_CONSTEXPR auto operator=(inplace_vector&& other) noexcept(N == 0 || (std::is_nothrow_move_assignable<T>::value &&
                                                                                      std::is_nothrow_move_constructible<T>::value))
        -> typename std::enable_if<std::is_move_constructible<U>::value, inplace_vector&>::type {
        clear();
        std::move(other.begin(), other.end(), std::back_inserter(*this));
        return *this;
    }

    template<class U = T>
    LYNIPV_CXX14_CONSTEXPR auto operator=(std::initializer_list<T> init) ->
        typename std::enable_if<std::is_copy_constructible<U>::value, inplace_vector&>::type {
        if(init.size() > capacity()) throw std::bad_alloc();
        assign(init.begin(), init.end());
        return *this;
    }

    template<class U = T>
    LYNIPV_CXX14_CONSTEXPR auto assign(size_type count, const T& value) ->
        typename std::enable_if<std::is_copy_constructible<U>::value>::type {
        if(count > capacity()) throw std::bad_alloc();
        clear();
        while(count != size()) push_back(value);
    }

    template<class InputIt, typename std::enable_if<
                                std::is_constructible<typename std::iterator_traits<InputIt>::value_type>::value, int>::type = 0>
    LYNIPV_CXX14_CONSTEXPR void assign(InputIt first, InputIt last) {
        clear();
        std::copy(first, last, std::back_inserter(*this));
    }

    LYNIPV_CXX14_CONSTEXPR void assign(std::initializer_list<T> ilist) {
        if(ilist.size() > capacity()) throw std::bad_alloc();
        clear();
        std::copy(ilist.begin(), ilist.end(), std::back_inserter(*this));
    }

#if __cplusplus >= 202002L
    template<detail::container_compatiblel_range<T> R>
    constexpr void assign_range(R&& rg)
        requires std::constructible_from<T&, std::ranges::range_reference_t<R>>
    {
        clear();
        append_range(std::forward<R>(rg));
    }

    template<detail::container_compatiblel_range<T> R>
    constexpr void append_range(R&& rg)
        requires std::constructible_from<T&, std::ranges::range_reference_t<R>>
    {
        if constexpr(std::ranges::sized_range<R>) {
            if(size() + std::ranges::size(rg) > capacity()) throw std::bad_alloc();
            for(auto&& val : rg) {
                unchecked_emplace_back(std::forward<decltype(val)>(val));
            }
        } else {
            for(auto&& val : rg) {
                emplace_back(std::forward<decltype(val)>(val));
            }
        }
    }

    template<detail::container_compatiblel_range<T> R>
    constexpr std::ranges::borrowed_iterator_t<R> try_append_range(R&& rg)
        requires std::constructible_from<T&, std::ranges::range_reference_t<R>>
    {
        auto it = std::ranges::begin(rg);
        for(auto end = std::ranges::end(rg); it != end; std::ranges::advance(it, 1)) {
            if(size() == capacity()) break;
            unchecked_emplace_back(*it);
        }
        return it;
    }
#endif

    // element access
    LYNIPV_CXX14_CONSTEXPR reference at(size_type idx) {
        if(idx >= size()) throw std::out_of_range("");
        return ref(idx);
    }
    LYNIPV_CXX14_CONSTEXPR const_reference at(size_type idx) const {
        if(idx >= size()) throw std::out_of_range("");
        return ref(idx);
    }
    LYNIPV_CXX14_CONSTEXPR reference front() { return ref(0); }
    constexpr const_reference front() const { return ref(0); }
    LYNIPV_CXX14_CONSTEXPR reference back() { return ref(size() - 1); }
    constexpr const_reference back() const { return ref(size() - 1); }
    LYNIPV_CXX14_CONSTEXPR pointer data() noexcept { return ptr(0); }
    LYNIPV_CXX14_CONSTEXPR const_pointer data() const noexcept { return ptr(0); }

    // iterators
    constexpr const_iterator cbegin() const noexcept { return data(); }
    constexpr const_iterator cend() const noexcept { return std::next(cbegin(), static_cast<difference_type>(size())); }
    constexpr const_iterator begin() const noexcept { return cbegin(); }
    constexpr const_iterator end() const noexcept { return cend(); }
    LYNIPV_CXX14_CONSTEXPR iterator begin() noexcept { return data(); }
    LYNIPV_CXX14_CONSTEXPR iterator end() noexcept { return std::next(begin(), static_cast<difference_type>(size())); }

    constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }
    constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }
    constexpr const_reverse_iterator rbegin() const noexcept { return crbegin(); }
    constexpr const_reverse_iterator rend() const noexcept { return crend(); }
    LYNIPV_CXX14_CONSTEXPR reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    LYNIPV_CXX14_CONSTEXPR reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

    // size and capacity
    constexpr bool empty() const noexcept { return size() == 0; }
    static constexpr size_type max_size() noexcept { return N; }
    static constexpr size_type capacity() noexcept { return N; }

private:
    LYNIPV_CXX14_CONSTEXPR void unchecked_resize(size_type count) {
        if(count < size()) {
            shrink_to(count);
        } else {
            while(count != size()) {
                unchecked_emplace_back();
            }
        }
    }
    LYNIPV_CXX14_CONSTEXPR void unchecked_resize(size_type count, const value_type& value) {
        if(count < size()) {
            shrink_to(count);
        } else {
            while(count != size()) {
                push_back_unchecked(value);
            }
        }
    }

public:
    LYNIPV_CXX14_CONSTEXPR void resize(size_type count) {
        if(count > capacity()) throw std::bad_alloc();
        unchecked_resize(count);
    }
    LYNIPV_CXX14_CONSTEXPR void resize(size_type count, const value_type& value) {
        if(count > capacity()) throw std::bad_alloc();
        unchecked_resize(count, value);
    }
    static LYNIPV_CXX14_CONSTEXPR void reserve(size_type new_cap) {
        if(new_cap > capacity()) throw std::bad_alloc();
    }
    static LYNIPV_CXX14_CONSTEXPR void shrink_to_fit() noexcept {}

    // modifiers
private:
    /*
    // optimization idea for all insert() functions to get away from constructing and rotating:
    LYNIPV_CXX14_CONSTEXPR size_type make_room_at(const_iterator pos, size_type count) {
        // - move construct some T's at current end().
        // - move assign some T's before current end().
        // - destroy the old host for those "moved from" but not "moved to".
        //
        // This should leave a nice gap to construct the new range in without the need for move assigning via rotate afterwards.
        //
        // I don't know what to do about exception guarantees with that implementation though so I'll leave it to something to think
        // about. Perhaps it can be used for non-throwing constructors of T.
    }
    */

public:
    LYNIPV_CXX14_CONSTEXPR iterator insert(const_iterator pos, const T& value) {
        static_assert(std::is_nothrow_move_assignable<T>::value, "only nothrow move assignable types may be used for now");
        if(size() == capacity()) throw std::bad_alloc();
        const auto ncpos = const_cast<iterator>(pos);
        unchecked_push_back(value);
        std::rotate(ncpos, std::prev(end()), end());
        return ncpos;
    }
    LYNIPV_CXX14_CONSTEXPR iterator insert(const_iterator pos, T&& value) {
        static_assert(std::is_nothrow_move_assignable<T>::value, "only nothrow move assignable types may be used for now");
        if(size() == capacity()) throw std::bad_alloc();
        const auto ncpos = const_cast<iterator>(pos);
        unchecked_push_back(std::move(value));
        std::rotate(ncpos, std::prev(end()), end());
        return ncpos;
    }
    LYNIPV_CXX20_CONSTEXPR iterator insert(const_iterator pos, size_type count, const T& value) {
        static_assert(std::is_nothrow_move_assignable<T>::value, "only nothrow move assignable types may be used for now");
        if(size() + count > capacity()) throw std::bad_alloc();
        const auto ncpos = const_cast<iterator>(pos);
        auto oldsize = size();
        auto first_inserted = end();
        try {
            while(count--) {
                unchecked_push_back(value);
            }
        } catch(...) {
            shrink_to(oldsize);
            throw;
        }
        std::rotate(ncpos, first_inserted, end());
        return ncpos;
    }
    template<class InputIt, typename std::enable_if<
                                std::is_constructible<typename std::iterator_traits<InputIt>::value_type>::value, int>::type = 0>
    LYNIPV_CXX20_CONSTEXPR iterator insert(const_iterator pos, InputIt first, InputIt last) {
        static_assert(std::is_nothrow_move_assignable<T>::value, "only nothrow move assignable types may be used for now");
        const auto ncpos = const_cast<iterator>(pos);
        auto oldsize = size();
        auto first_inserted = end();
        try {
            for(; first != last; std::advance(first, 1)) {
                push_back(*first);
            }
        } catch(...) {
            shrink_to(oldsize);
            throw;
        }
        std::rotate(ncpos, first_inserted, end());
        return ncpos;
    }
    LYNIPV_CXX14_CONSTEXPR iterator insert(const_iterator pos, std::initializer_list<T> ilist) {
        return insert(pos, ilist.begin(), ilist.end());
    }

    template<class... Args>
    LYNIPV_CXX14_CONSTEXPR iterator emplace(const_iterator pos, Args&&... args) {
        static_assert(std::is_nothrow_move_assignable<T>::value, "only nothrow move assignable types may be used for now");
        const auto ncpos = const_cast<iterator>(pos);
        emplace_back(std::forward<Args>(args)...);
        std::rotate(ncpos, std::prev(end()), end());
        return ncpos;
    }

    template<class... Args>
    LYNIPV_CXX14_CONSTEXPR reference unchecked_emplace_back(Args&&... args) {
        auto& rv = construct(size(), std::forward<Args>(args)...);
        this->inc();
        return rv;
    }
    LYNIPV_CXX14_CONSTEXPR reference unchecked_push_back(T const& value) {
        auto& rv = construct(size(), value);
        this->inc();
        return rv;
    }
    LYNIPV_CXX14_CONSTEXPR reference unchecked_push_back(T&& value) {
        auto& rv = construct(size(), std::move(value));
        this->inc();
        return rv;
    }

    template<class... Args>
    LYNIPV_CXX14_CONSTEXPR reference emplace_back(Args&&... args) {
        if(size() == N) throw std::bad_alloc();
        return unchecked_emplace_back(std::forward<Args>(args)...);
    }
    template<class... Args>
    LYNIPV_CXX14_CONSTEXPR pointer try_emplace_back(Args&&... args) {
        if(size() == N) return nullptr;
        return std::addressof(unchecked_emplace_back(std::forward<Args>(args)...));
    }

    LYNIPV_CXX14_CONSTEXPR reference push_back(T const& value) {
        if(size() == N) throw std::bad_alloc();
        return unchecked_push_back(value);
    }
    LYNIPV_CXX14_CONSTEXPR reference push_back(T&& value) {
        if(size() == N) throw std::bad_alloc();
        return unchecked_push_back(std::move(value));
    }
    LYNIPV_CXX14_CONSTEXPR pointer try_push_back(T const& value) {
        if(size() == N) return nullptr;
        return std::addressof(unchecked_push_back(value));
    }
    LYNIPV_CXX14_CONSTEXPR pointer try_push_back(T&& value) {
        if(size() == N) return nullptr;
        return std::addressof(unchecked_push_back(std::move(value)));
    }

    LYNIPV_CXX14_CONSTEXPR void pop_back() noexcept { destroy(this->dec()); }
    LYNIPV_CXX14_CONSTEXPR void clear() noexcept { shrink_to(0); }

    LYNIPV_CXX14_CONSTEXPR iterator erase(const_iterator first, const_iterator last) {
        auto ncfirst = const_cast<iterator>(first);
        auto nclast = const_cast<iterator>(last);
        auto removed = static_cast<std::size_t>(std::distance(ncfirst, nclast));
        std::move(nclast, end(), ncfirst);
        for(size_type idx = size() - removed; idx < size(); ++idx) {
            destroy(idx);
        }
        this->dec(removed);
        return ncfirst;
    }
    LYNIPV_CXX14_CONSTEXPR iterator erase(const_iterator pos) { return erase(pos, std::next(pos)); }

    LYNIPV_CXX14_CONSTEXPR void swap(inplace_vector& other) noexcept(N == 0 || (cpp26::is_nothrow_swappable<T>::value &&
                                                                                std::is_nothrow_move_constructible<T>::value)) {
        auto&& p = (size() < other.size()) ? std::pair<inplace_vector&, inplace_vector&>(*this, other)
                                           : std::pair<inplace_vector&, inplace_vector&>(other, *this);
        auto& small = p.first;
        auto& large = p.second;
        size_type idx = 0, small_size = small.size();
        for(; idx < small_size; ++idx) {
            using std::swap;
            swap(small[idx], large[idx]);
        }
        for(; idx < large.size(); ++idx) {
            small.push_back(std::move(large[idx]));
        }
        large.shrink_to(small_size);
    }
    LYNIPV_CXX14_CONSTEXPR void friend swap(inplace_vector& lhs, inplace_vector& rhs) noexcept(
        N == 0 || (cpp26::is_nothrow_swappable<T>::value && std::is_nothrow_move_constructible<T>::value)) {
        lhs.swap(rhs);
    }

#if __cplusplus >= 202002L
    constexpr friend auto operator<=>(const inplace_vector& lhs, const inplace_vector& rhs) {
        return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }
#else
    friend bool operator<(const inplace_vector& lhs, const inplace_vector& rhs) {
        return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }
    friend bool operator>(const inplace_vector& lhs, const inplace_vector& rhs) { return rhs < lhs; }
    friend bool operator<=(const inplace_vector& lhs, const inplace_vector& rhs) { return !(rhs < lhs); }
    friend bool operator>=(const inplace_vector& lhs, const inplace_vector& rhs) { return rhs <= lhs; }
    friend bool operator!=(const inplace_vector& lhs, const inplace_vector& rhs) { return !(lhs == rhs); }
#endif
    friend bool operator==(const inplace_vector& lhs, const inplace_vector& rhs) {
        if(lhs.size() != rhs.size()) return false;
        return std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin());
    }
};

} // namespace cpp26

#undef LYNIPV_CONSTRUCT_AT
#endif
