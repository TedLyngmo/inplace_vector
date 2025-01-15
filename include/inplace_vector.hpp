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
#include <initializer_list>
#include <iterator>
#include <new>
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

#ifndef LYNIPV_LAUNDER
# if __cplusplus >= 201703L
#  define LYNIPV_LAUNDER(x) std::launder(x)
# else
#  define LYNIPV_LAUNDER(x) x
# endif
#endif

namespace cpp26 {

#if __cplusplus >= 201703L
using std::is_nothrow_swappable;
#else
template<typename U>
struct is_nothrow_swappable : std::integral_constant<bool, noexcept(swap(std::declval<U&>(), std::declval<U&>()))> {};
#endif

enum class byte : unsigned char {};

template<class T, std::size_t N>
class inplace_vector {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = T const&;
    using pointer = T*;
    using const_pointer = T const*;
    using iterator = T*;
    using const_iterator = T const*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    struct alignas(T) aligned_storage {
        LYNIPV_CXX14_CONSTEXPR reference ref() { return *LYNIPV_LAUNDER(reinterpret_cast<pointer>(m_raw)); }
        LYNIPV_CXX14_CONSTEXPR const_reference ref() const { return *LYNIPV_LAUNDER(reinterpret_cast<const_pointer>(m_raw)); }

        template<class... Args>
        LYNIPV_CXX14_CONSTEXPR reference construct(Args&&... args) {
            return *::new(static_cast<void*>(m_raw)) T(std::forward<Args>(args)...);
        }
        LYNIPV_CXX14_CONSTEXPR void destroy() { ref().~T(); }
        byte m_raw[sizeof(T)];
    };

    LYNIPV_CXX14_CONSTEXPR void shrink_by(size_type count) noexcept {
        for(size_type idx = m_size - count; idx < m_size; ++idx) {
            m_data[idx].destroy();
        }
        m_size -= count;
    }

public:
    // constructors
    constexpr inplace_vector() noexcept = default;
    LYNIPV_CXX14_CONSTEXPR explicit inplace_vector(size_type count) {
        if(count > N) throw std::bad_alloc();
        while(count--) unchecked_emplace_back();
    }
    LYNIPV_CXX14_CONSTEXPR inplace_vector(size_type count, const T& value) {
        if(count > N) throw std::bad_alloc();
        while(count--) unchecked_push_back(value);
    }
    template<class InputIt>
    LYNIPV_CXX14_CONSTEXPR inplace_vector(InputIt first, InputIt last) {
        std::copy(first, last, std::back_inserter(*this));
    }

    // make copy/move use push_back_unchecked
    constexpr inplace_vector(const inplace_vector& other) : inplace_vector(other.begin(), other.end()) {}

    LYNIPV_CXX14_CONSTEXPR inplace_vector(inplace_vector&& other) noexcept(N == 0 || std::is_nothrow_move_constructible<T>::value) {
        std::move(other.begin(), other.end(), std::back_inserter(*this));
    }

    // this needs to use the checked version though
    constexpr inplace_vector(std::initializer_list<T> init) : inplace_vector(init.begin(), init.end()) {}

    // destructor
    LYNIPV_CXX20_CONSTEXPR ~inplace_vector() noexcept { clear(); }

    // assignment
    LYNIPV_CXX14_CONSTEXPR inplace_vector& operator=(const inplace_vector& other) {
        assign(other.begin(), other.end());
        return *this;
    }
    LYNIPV_CXX14_CONSTEXPR inplace_vector& operator=(inplace_vector&& other) noexcept(
        N == 0 || (std::is_nothrow_move_assignable<T>::value && std::is_nothrow_move_constructible<T>::value)) {
        clear();
        std::move(other.begin(), other.end(), std::back_inserter(*this));
        return *this;
    }
    LYNIPV_CXX14_CONSTEXPR inplace_vector& operator=(std::initializer_list<T> init) {
        if(init.size() > capacity()) throw std::bad_alloc();
        assign(init.begin(), init.end());
        return *this;
    }
    LYNIPV_CXX14_CONSTEXPR void assign(size_type count, const T& value) {
        if(count > capacity()) throw std::bad_alloc();
        clear();
        while(count--) push_back(value);
    }
    template<class InputIt>
    LYNIPV_CXX14_CONSTEXPR void assign(InputIt first, InputIt last) {
        clear();
        std::copy(first, last, std::back_inserter(*this));
    }
    LYNIPV_CXX14_CONSTEXPR void assign(std::initializer_list<T> ilist) {
        clear();
        std::copy(ilist.begin(), ilist.end(), std::back_inserter(*this));
    }

    // element access
    LYNIPV_CXX14_CONSTEXPR reference at(size_type idx) {
        if(idx >= m_size) throw std::out_of_range("");
        return m_data[idx].ref();
    }
    LYNIPV_CXX14_CONSTEXPR const_reference at(size_type idx) const {
        if(idx >= m_size) throw std::out_of_range("");
        return m_data[idx].ref();
    }
    LYNIPV_CXX14_CONSTEXPR reference operator[](size_type idx) noexcept { return m_data[idx].ref(); }
    constexpr const_reference operator[](size_type idx) const noexcept { return m_data[idx].ref(); }
    LYNIPV_CXX14_CONSTEXPR reference front() { return m_data[0].ref(); }
    constexpr const_reference front() const { return m_data[0].ref(); }
    LYNIPV_CXX14_CONSTEXPR reference back() { return m_data[m_size - 1].ref(); }
    constexpr const_reference back() const { return m_data[m_size - 1].ref(); }
    LYNIPV_CXX14_CONSTEXPR pointer data() noexcept { return &m_data[0].ref(); }
    LYNIPV_CXX14_CONSTEXPR const_pointer data() const noexcept { return &m_data[0].ref(); }

    // iterators
    constexpr const_iterator cbegin() const noexcept { return data(); }
    constexpr const_iterator cend() const noexcept { return std::next(cbegin(), static_cast<difference_type>(m_size)); }
    constexpr const_iterator begin() const noexcept { return cbegin(); }
    constexpr const_iterator end() const noexcept { return cend(); }
    LYNIPV_CXX14_CONSTEXPR iterator begin() noexcept { return data(); }
    LYNIPV_CXX14_CONSTEXPR iterator end() noexcept { return std::next(begin(), static_cast<difference_type>(m_size)); }

    constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }
    constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }
    constexpr const_reverse_iterator rbegin() const noexcept { return crbegin(); }
    constexpr const_reverse_iterator rend() const noexcept { return crend(); }
    LYNIPV_CXX14_CONSTEXPR reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    LYNIPV_CXX14_CONSTEXPR reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

    // size and capacity
    constexpr bool empty() const noexcept { return m_size == 0; }
    constexpr size_type size() const noexcept { return m_size; }
    static constexpr size_type max_size() noexcept { return N; }
    static constexpr size_type capacity() noexcept { return N; }

private:
    LYNIPV_CXX14_CONSTEXPR void unchecked_resize(size_type count) {
        if(count < size()) {
            shrink_by(size() - count);
        } else {
            count -= size();
            while(count--) {
                unchecked_emplace_back();
            }
        }
    }
    LYNIPV_CXX14_CONSTEXPR void unchecked_resize(size_type count, const value_type& value) {
        if(count < size()) {
            shrink_by(size() - count);
        } else {
            count -= size();
            while(count--) {
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
    LYNIPV_CXX14_CONSTEXPR size_type make_room_at(const_iterator pos, size_type count) {
        // optimization idea for all insert() functions to get away from constructing and rotating:
        // create a gap by move constructing count T's after end.
        // move assign size()-count-distance(begin(),pos) T' that are before that.
        // ... and destroy the old host for those move assigned.
        // This should leave a nice gap to construct the new range in.
        // I don't know what to do about exception guarantees with that implementation though.
        return {};
    }

public:
    LYNIPV_CXX14_CONSTEXPR iterator insert(const_iterator pos, const T& value) {
        static_assert(std::is_nothrow_move_assignable<T>::value, "only nothrow move assignable types may be used for now");
        if(size() == capacity()) throw std::bad_alloc();
        const auto ncpos = const_cast<iterator>(pos);
        unchecked_emplace_back(value);
        std::rotate(ncpos, std::prev(end()), end());
        return ncpos;
    }
    LYNIPV_CXX14_CONSTEXPR iterator insert(const_iterator pos, T&& value) {
        static_assert(std::is_nothrow_move_assignable<T>::value, "only nothrow move assignable types may be used for now");
        if(size() == capacity()) throw std::bad_alloc();
        const auto ncpos = const_cast<iterator>(pos);
        unchecked_emplace_back(std::move(value));
        std::rotate(ncpos, std::prev(end()), end());
        return ncpos;
    }
    LYNIPV_CXX20_CONSTEXPR iterator insert(const_iterator pos, size_type count, const T& value) {
        static_assert(std::is_nothrow_move_assignable<T>::value, "only nothrow move assignable types may be used for now");
        if(size() + count > capacity()) throw std::bad_alloc();
        const auto ncpos = const_cast<iterator>(pos);
        auto oldsize = size();
        try {
            while(count--) {
                unchecked_emplace_back(value);
            }
        } catch(...) {
            unchecked_resize(oldsize);
            throw;
        }
        std::rotate(ncpos, std::prev(end()), end());
        return ncpos;
    }
    template<class InputIt>
    LYNIPV_CXX20_CONSTEXPR iterator insert(const_iterator pos, InputIt first, InputIt last) {
        static_assert(std::is_nothrow_move_assignable<T>::value, "only nothrow move assignable types may be used for now");
        const auto ncpos = const_cast<iterator>(pos);
        auto oldsize = size();
        try {
            for(; first != last; ++first) {
                unchecked_emplace_back(*first);
            }
        } catch(...) {
            unchecked_resize(oldsize);
            throw;
        }
        std::rotate(ncpos, std::prev(end(), size() - oldsize), end());
        return ncpos;
    }
    LYNIPV_CXX14_CONSTEXPR iterator insert(const_iterator pos, std::initializer_list<T> ilist) {
        return insert(pos, ilist.begin(), ilist.end());
    }
    /*
    template< class... Args >
    LYNIPV_CXX14_CONSTEXPR iterator emplace( const_iterator position, Args&&... args );
    */
    template<class... Args>
    LYNIPV_CXX14_CONSTEXPR reference unchecked_emplace_back(Args&&... args) {
        auto& rv = m_data[m_size].construct(std::forward<Args>(args)...);
        ++m_size;
        return rv;
    }
    LYNIPV_CXX14_CONSTEXPR reference unchecked_push_back(T const& value) {
        auto& rv = m_data[m_size].construct(value);
        ++m_size;
        return rv;
    }
    LYNIPV_CXX14_CONSTEXPR reference unchecked_push_back(T&& value) {
        auto& rv = m_data[m_size].construct(std::move(value));
        ++m_size;
        return rv;
    }

    template<class... Args>
    LYNIPV_CXX14_CONSTEXPR reference emplace_back(Args&&... args) {
        if(m_size == N) throw std::bad_alloc();
        return unchecked_emplace_back(std::forward<Args>(args)...);
    }
    LYNIPV_CXX14_CONSTEXPR reference push_back(T const& value) {
        if(m_size == N) throw std::bad_alloc();
        return unchecked_push_back(value);
    }
    LYNIPV_CXX14_CONSTEXPR reference push_back(T&& value) {
        if(m_size == N) throw std::bad_alloc();
        return unchecked_push_back(std::move(value));
    }

    LYNIPV_CXX14_CONSTEXPR void pop_back() noexcept { shrink_by(1); }
    LYNIPV_CXX14_CONSTEXPR void clear() noexcept { shrink_by(size()); }

    LYNIPV_CXX14_CONSTEXPR iterator erase(const_iterator first, const_iterator last) {
        auto ncfirst = const_cast<iterator>(first);
        auto nclast = const_cast<iterator>(last);
        auto removed = std::distance(ncfirst, nclast);
        std::move(nclast, end(), ncfirst);
        for(size_type idx = m_size - removed; idx < m_size; ++idx) {
            m_data[idx].destroy();
        }
        m_size -= removed;
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
        large.unchecked_resize(small_size);
    }
    LYNIPV_CXX14_CONSTEXPR void friend swap(inplace_vector& lhs, inplace_vector& rhs) noexcept(
        N == 0 || (cpp26::is_nothrow_swappable<T>::value && std::is_nothrow_move_constructible<T>::value)) {
        lhs.swap(rhs);
    }

private:
    std::array<aligned_storage, N> m_data;
    size_type m_size = 0;
};

} // namespace cpp26
#endif
