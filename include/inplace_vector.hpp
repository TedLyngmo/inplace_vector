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
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

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
        constexpr reference ref() { return *reinterpret_cast<pointer>(m_raw); }
        constexpr const_reference ref() const { return *std::launder(reinterpret_cast<const_pointer>(m_raw)); }

        template<class... Args>
        constexpr reference construct(Args&&... args) {
            return *::new(static_cast<void*>(m_raw)) T(std::forward<Args>(args)...);
        }
        constexpr void destroy() { ref().~T(); }
        std::byte m_raw[sizeof(T)];
    };

    constexpr void destroy_all() noexcept {
        for(std::size_t idx = 0; idx < m_size; ++idx) {
            m_data[idx].destroy();
        }
    }

public:
    constexpr inplace_vector() noexcept = default;
    constexpr explicit inplace_vector(size_type count) {
        while(count--) emplace_back();
    }
    constexpr inplace_vector(size_type count, const T& value) {
        while(count--) push_back(value);
    }
    template<class InputIt>
    constexpr inplace_vector(InputIt first, InputIt last) {
        std::copy(first, last, std::back_inserter(*this));
        m_size = std::distance(first, last);
    }

    constexpr inplace_vector(const inplace_vector& other) : inplace_vector(other.begin(), other.end()) {}

    constexpr inplace_vector(inplace_vector&& other) noexcept(N == 0 || std::is_nothrow_move_constructible_v<T>) :
        inplace_vector(std::move_iterator(other.begin()), std::move_iterator(other.end())) {}

    constexpr inplace_vector(std::initializer_list<T> init) : inplace_vector(init.begin(), init.end()) {}

    ~inplace_vector() noexcept { destroy_all(); }
    constexpr void clear() noexcept {
        destroy_all();
        m_size = 0;
    }
    constexpr std::size_t size() const noexcept { return m_size; }
    constexpr std::size_t capacity() const noexcept { return N; }
    constexpr bool empty() const noexcept { return m_size == 0; }

    template<class... Args>
    constexpr reference emplace_back(Args&&... args) {
        if(m_size == N) throw std::bad_alloc();
        auto& rv = m_data[m_size].construct(std::forward<Args>(args)...);
        m_size++;
        return rv;
    }
    constexpr reference push_back(T const& value) {
        if(m_size == N) throw std::bad_alloc();
        auto& rv = m_data[m_size].construct(value);
        m_size++;
        return rv;
    }
    constexpr reference push_back(T&& value) {
        if(m_size == N) throw std::bad_alloc();
        auto& rv = m_data[m_size].construct(std::move(value));
        m_size++;
        return rv;
    }
    constexpr reference operator[](std::size_t idx) noexcept { return m_data[idx].ref(); }
    constexpr const_reference operator[](std::size_t idx) const noexcept { return m_data[idx].ref(); }
    constexpr reference at(std::size_t idx) {
        if(idx >= m_size) throw std::out_of_range("");
        return m_data[idx].ref();
    }
    constexpr const_reference at(std::size_t idx) const {
        if(idx >= m_size) throw std::out_of_range("");
        return m_data[idx].ref();
    }
    constexpr iterator erase(const_iterator first, const_iterator last) {
        auto ncfirst = const_cast<iterator>(first);
        auto nclast = const_cast<iterator>(last);
        auto removed = std::distance(ncfirst, nclast);
        std::move(nclast, end(), ncfirst);
        for(size_t idx = m_size - removed; idx < m_size; ++idx) {
            m_data[idx].destroy();
        }
        m_size -= removed;
        return ncfirst;
    }
    constexpr iterator erase(const_iterator pos) { return erase(pos, std::next(pos)); }

    constexpr void swap(inplace_vector& other) noexcept(N == 0 || (std::is_nothrow_swappable_v<T> &&
                                                                   std::is_nothrow_move_constructible_v<T>)) {
        // TODO
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
    std::size_t m_size = 0;
};

} // namespace cpp26
#endif
