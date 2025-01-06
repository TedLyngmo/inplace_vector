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
#ifndef INPLACE_VECTOR_HPP_F4BA9AA8_99CD_11EF_8916_90B11C0C0FF8
#define INPLACE_VECTOR_HPP_F4BA9AA8_99CD_11EF_8916_90B11C0C0FF8

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

#if __cplusplus >= 202002L
# define INPLACE_VECTOR_HPP_CPP20CONSTEXPR constexpr
#else
# define INPLACE_VECTOR_HPP_CPP20CONSTEXPR
#endif

namespace cpp26 {

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
        constexpr reference ref() { return *std::launder(reinterpret_cast<pointer>(m_raw)); }
        constexpr const_reference ref() const { return *std::launder(reinterpret_cast<const_pointer>(m_raw)); }

        template<class... Args>
        constexpr reference construct(Args&&... args) {
            return *::new(static_cast<void*>(m_raw)) T(std::forward<Args>(args)...);
        }
        constexpr void destroy() { ref().~T(); }
        std::byte m_raw[sizeof(T)];
    };

    constexpr void shrink_by(size_type count) noexcept {
        for(size_type idx = m_size - count; idx < m_size; ++idx) {
            m_data[idx].destroy();
        }
        m_size -= count;
    }

public:
    // constructors
    constexpr inplace_vector() noexcept = default;
    constexpr explicit inplace_vector(size_type count) {
        if(count > N) throw std::bad_alloc();
        while(count--) emplace_back_unchecked();
    }
    constexpr inplace_vector(size_type count, const T& value) {
        if(count > N) throw std::bad_alloc();
        while(count--) push_back_unchecked(value);
    }
    template<class InputIt>
    constexpr inplace_vector(InputIt first, InputIt last) {
        std::copy(first, last, std::back_inserter(*this));
    }

    // make copy/move use push_back_unchecked
    constexpr inplace_vector(const inplace_vector& other) : inplace_vector(other.begin(), other.end()) {}

    constexpr inplace_vector(inplace_vector&& other) noexcept(N == 0 || std::is_nothrow_move_constructible_v<T>) {
        std::move(other.begin(), other.end(), std::back_inserter(*this));
    }

    // this needs to use the checked version though
    constexpr inplace_vector(std::initializer_list<T> init) : inplace_vector(init.begin(), init.end()) {}

    // destructor
    INPLACE_VECTOR_HPP_CPP20CONSTEXPR ~inplace_vector() noexcept { clear(); }

    // assignment
    constexpr inplace_vector& operator=(const inplace_vector& other) {
        assign(other.begin(), other.end());
        return *this;
    }
    constexpr inplace_vector& operator=(inplace_vector&& other) noexcept(N == 0 || (std::is_nothrow_move_assignable_v<T> &&
                                                                                    std::is_nothrow_move_constructible_v<T>)) {
        clear();
        std::move(other.begin(), other.end(), std::back_inserter(*this));
        return *this;
    }
    constexpr inplace_vector& operator=(std::initializer_list<T> init) {
        if(init.size() > capacity()) throw std::bad_alloc();
        assign(init.begin(), init.end());
        return *this;
    }
    constexpr void assign(size_type count, const T& value) {
        if(count > capacity()) throw std::bad_alloc();
        clear();
        while(count--) push_back(value);
    }
    template<class InputIt>
    constexpr void assign(InputIt first, InputIt last) {
        clear();
        std::copy(first, last, std::back_inserter(*this));
    }
    constexpr void assign(std::initializer_list<T> ilist) {
        clear();
        std::copy(ilist.begin(), ilist.end(), std::back_inserter(*this));
    }

    //
    constexpr bool empty() const noexcept { return m_size == 0; }
    constexpr size_type size() const noexcept { return m_size; }
    static constexpr size_type max_size() noexcept { return N; }
    static constexpr size_type capacity() noexcept { return N; }

private:
    constexpr void resize_unchecked(size_type count) {
        if(count < size()) {
            shrink_by(size() - count);
        } else {
            count -= size();
            while(count--) {
                emplace_back_unchecked();
            }
        }
    }
    constexpr void resize_unchecked(size_type count, const value_type& value) {
        if(count < size()) {
            shrink_by(size() - count);
        } else {
            count -= size();
            while(count--) {
                push_back_unchecked(value);
            }
        }
    }

private:
    constexpr void resize(size_type count) {
        if(count > capacity()) throw std::bad_alloc();
        resize_unchecked(count);
    }
    constexpr void resize(size_type count, const value_type& value) {
        if(count > capacity()) throw std::bad_alloc();
        resize_unchecked(count, value);
    }

private:
    template<class... Args>
    constexpr reference emplace_back_unchecked(Args&&... args) {
        auto& rv = m_data[m_size].construct(std::forward<Args>(args)...);
        ++m_size;
        return rv;
    }
    constexpr reference push_back_unchecked(T const& value) {
        auto& rv = m_data[m_size].construct(value);
        ++m_size;
        return rv;
    }
    constexpr reference push_back_unchecked(T&& value) {
        auto& rv = m_data[m_size].construct(std::move(value));
        ++m_size;
        return rv;
    }

public:
    template<class... Args>
    constexpr reference emplace_back(Args&&... args) {
        if(m_size == N) throw std::bad_alloc();
        return emplace_back_unchecked(std::forward<Args>(args)...);
    }
    constexpr reference push_back(T const& value) {
        if(m_size == N) throw std::bad_alloc();
        return push_back_unchecked(value);
    }
    constexpr reference push_back(T&& value) {
        if(m_size == N) throw std::bad_alloc();
        return push_back_unchecked(std::move(value));
    }
    constexpr reference operator[](size_type idx) noexcept { return m_data[idx].ref(); }
    constexpr const_reference operator[](size_type idx) const noexcept { return m_data[idx].ref(); }
    constexpr reference at(size_type idx) {
        if(idx >= m_size) throw std::out_of_range("");
        return m_data[idx].ref();
    }
    constexpr const_reference at(size_type idx) const {
        if(idx >= m_size) throw std::out_of_range("");
        return m_data[idx].ref();
    }

    constexpr void pop_back() noexcept { shrink_by(1); }

    constexpr void clear() noexcept { shrink_by(size()); }

    constexpr iterator erase(const_iterator first, const_iterator last) {
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
    constexpr iterator erase(const_iterator pos) { return erase(pos, std::next(pos)); }

    constexpr void swap(inplace_vector& other) noexcept(N == 0 || (std::is_nothrow_swappable_v<T> &&
                                                                   std::is_nothrow_move_constructible_v<T>)) {
        auto&& [small, large] = (size() < other.size()) ? std::pair<inplace_vector&, inplace_vector&>(*this, other)
                                                        : std::pair<inplace_vector&, inplace_vector&>(other, *this);
        size_type idx = 0, small_size = small.size();
        for(; idx < small_size; ++idx) {
            using std::swap;
            swap(small[idx], large[idx]);
        }
        for(; idx < large.size(); ++idx) {
            small.push_back(std::move(large[idx]));
        }
        large.resize_unchecked(small_size);
    }
    constexpr void friend swap(inplace_vector& lhs,
                               inplace_vector& rhs) noexcept(N == 0 || (std::is_nothrow_swappable_v<T> &&
                                                                        std::is_nothrow_move_constructible_v<T>)) {
        lhs.swap(rhs);
    }

    constexpr reference front() { return m_data[0].ref(); }
    constexpr const_reference front() const { return m_data[0].ref(); }
    constexpr reference back() { return m_data[m_size - 1].ref(); }
    constexpr const_reference back() const { return m_data[m_size - 1].ref(); }

    constexpr const_iterator cbegin() const noexcept { return &m_data[0].ref(); }
    constexpr const_iterator cend() const noexcept { return &m_data[0].ref() + m_size; }
    constexpr const_iterator begin() const noexcept { return cbegin(); }
    constexpr const_iterator end() const noexcept { return cend(); }
    constexpr iterator begin() noexcept { return &m_data[0].ref(); }
    constexpr iterator end() noexcept { return &m_data[0].ref() + m_size; }

private:
    std::array<aligned_storage, N> m_data;
    size_type m_size = 0;
};

} // namespace cpp26
#endif
