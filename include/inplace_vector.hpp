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
#include <cstring>
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

namespace lyn {

template<class, std::size_t>
class inplace_vector;

namespace lyn_inplace_vector_detail {
#if __cplusplus >= 201402L
# define LYNIPV_CXX14_CONSTEXPR constexpr
#else
# define LYNIPV_CXX14_CONSTEXPR
#endif

#if __cplusplus >= 201703L
# define LYNIPV_LAUNDER(x) std::launder(x)
    using std::is_nothrow_swappable;
    template<class... B>
    using conjunction = std::conjunction<B...>;
#else
# define LYNIPV_LAUNDER(x) x
    template<typename U>
    struct is_nothrow_swappable : std::integral_constant<bool, noexcept(swap(std::declval<U&>(), std::declval<U&>()))> {};

    template<class...>
    struct conjunction : std::true_type {};

    template<class B1>
    struct conjunction<B1> : B1 {};

    template<class B1, class... Bn>
    struct conjunction<B1, Bn...> : std::conditional<bool(B1::value), conjunction<Bn...>, B1>::type {};
#endif

#if __cplusplus >= 202002L
# define LYNIPV_CXX20_CONSTEXPR constexpr
# define LYNIPV_CONSTRUCT_AT(p, ...) std::construct_at(p __VA_OPT__(, ) __VA_ARGS__)
    template<class R, class T>
    concept container_compatiblel_range = std::ranges::input_range<R> && std::convertible_to<std::ranges::range_reference_t<R>, T>;
#else
# define LYNIPV_CXX20_CONSTEXPR
# define LYNIPV_CONSTRUCT_AT(p, ...) ::new(static_cast<void*>(p)) T(__VA_ARGS__)
#endif

#if defined(LYNIPV_DEBUG) && __cplusplus >= 201703L
    template<class T>
    struct LynIpvDebug {
        template<class... Args>
        std::string cre(Args&&... args) {
            std::ostringstream oss;
            (..., (oss << ' ' << std::forward<Args>(args)));
            return oss.str();
        }
        template<class... Args>
        LynIpvDebug(Args&&... args) : m_debug(cre(std::forward<Args>(args)...)) {
            std::cout << "ENTER:" << m_debug << '\n';
        }
        ~LynIpvDebug() { std::cout << "EXIT: " << m_debug << '\n'; }
        std::string m_debug;
    };
# define TRACE_ENTER(...) lyn::lyn_inplace_vector_detail::LynIpvDebug<T> kdhfgkljshdfgkljsdflgkh(__VA_ARGS__)
#else
# define TRACE_ENTER(...)
#endif

    // base requirements
    template<class T, std::size_t N>
    struct constexpr_compat :
        std::integral_constant<
            bool, N == 0 || (std::is_trivially_default_constructible<T>::value && std::is_trivially_copyable<T>::value &&
                             // extra req. for now:
                             std::is_trivially_copy_constructible<T>::value && std::is_trivially_move_constructible<T>::value &&
                             std::is_trivially_copy_assignable<T>::value && std::is_trivially_move_assignable<T>::value &&
                             std::is_trivially_destructible<T>::value)> {};

    template<class T, std::size_t N>
    struct trivial_copy_ctor :
        std::integral_constant<bool,
                               N == 0 || (std::is_copy_constructible<T>::value && std::is_trivially_copy_constructible<T>::value)> {
    };

    template<class T, std::size_t N>
    struct trivial_move_ctor :
        std::integral_constant<bool,
                               N == 0 || (std::is_move_constructible<T>::value && std::is_trivially_move_constructible<T>::value)> {
    };

    template<class T, std::size_t N>
    struct trivial_copy_ass :
        std::integral_constant<bool, N == 0 || (std::is_trivially_destructible<T>::value &&
                                                std::is_trivially_copy_constructible<T>::value &&
                                                std::is_trivially_copy_assignable<T>::value)> {};

    template<class T, std::size_t N>
    struct trivial_move_ass :
        std::integral_constant<bool, N == 0 || (std::is_trivially_destructible<T>::value &&
                                                std::is_trivially_move_constructible<T>::value &&
                                                std::is_trivially_move_assignable<T>::value)> {};

    template<class T>
    struct aligned_storage_empty { // specialization for 0 elements
        using value_type = typename std::remove_const<T>::type;
        using size_type = std::size_t;
        using reference = value_type&;
        using const_reference = value_type const&;
        using pointer = value_type*;
        using const_pointer = value_type const*;

    protected:
        LYNIPV_CXX14_CONSTEXPR pointer ptr(size_type) { return nullptr; }
        LYNIPV_CXX14_CONSTEXPR const_pointer ptr(size_type) const { return nullptr; }
        LYNIPV_CXX14_CONSTEXPR reference ref(size_type) { return *ptr(0); }
        LYNIPV_CXX14_CONSTEXPR const_reference ref(size_type) const { return *ptr(0); }

        template<class... Args>
        LYNIPV_CXX20_CONSTEXPR reference construct_back(Args&&...) {
            return *ptr(0);
        }
        LYNIPV_CXX14_CONSTEXPR void destroy(size_type) {}

        LYNIPV_CXX14_CONSTEXPR reference operator[](size_type) { return *ptr(0); }
        constexpr const_reference operator[](size_type) const { return *ptr(0); }

        constexpr size_type size() const noexcept { return 0; }
        LYNIPV_CXX14_CONSTEXPR void clear() noexcept {}

        LYNIPV_CXX14_CONSTEXPR size_type inc() { return 0; }
        LYNIPV_CXX14_CONSTEXPR size_type dec(size_type = 1) { return 0; }
    };

    template<class T, std::size_t N>
    struct aligned_storage_trivial {
        static_assert(std::is_trivially_destructible<T>::value, "T must be trivially destructible");

        using value_type = typename std::remove_const<T>::type;
        using size_type = std::size_t;
        using reference = value_type&;
        using const_reference = value_type const&;
        using pointer = value_type*;
        using const_pointer = value_type const*;

    protected:
        LYNIPV_CXX14_CONSTEXPR pointer ptr(size_type idx) noexcept { return std::addressof(m_data[idx]); }
        LYNIPV_CXX14_CONSTEXPR const_pointer ptr(size_type idx) const noexcept { return std::addressof(m_data[idx]); }
        LYNIPV_CXX14_CONSTEXPR reference ref(size_type idx) noexcept { return m_data[idx]; }
        LYNIPV_CXX14_CONSTEXPR const_reference ref(size_type idx) const noexcept { return m_data[idx]; }

        template<class... Args>
        LYNIPV_CXX20_CONSTEXPR reference construct_back(Args&&... args) {
            auto& rv = m_data[m_size] = value_type{std::forward<Args>(args)...};
            ++m_size;
            return rv;
        }
        LYNIPV_CXX14_CONSTEXPR void destroy(size_type) noexcept {}

        LYNIPV_CXX14_CONSTEXPR reference operator[](size_type idx) noexcept { return ref(idx); }
        constexpr const_reference operator[](size_type idx) const noexcept { return ref(idx); }

        constexpr size_type size() const noexcept { return m_size; }
        LYNIPV_CXX14_CONSTEXPR void clear() noexcept { m_size = 0; }

        LYNIPV_CXX14_CONSTEXPR size_type inc() noexcept { return ++m_size; }
        LYNIPV_CXX14_CONSTEXPR size_type dec(size_type count = 1) noexcept { return m_size -= count; }

    private:
        std::array<value_type, N> m_data;
        size_type m_size = 0;
    };

    template<class T, std::size_t N>
    struct aligned_storage_non_trivial {
        using value_type = typename std::remove_const<T>::type;
        using size_type = std::size_t;
        using reference = value_type&;
        using const_reference = value_type const&;
        using pointer = value_type*;
        using const_pointer = value_type const*;

    protected:
        LYNIPV_CXX14_CONSTEXPR pointer ptr(size_type idx) noexcept { return m_data[idx].ptr(); }
        LYNIPV_CXX14_CONSTEXPR const_pointer ptr(size_type idx) const noexcept { return m_data[idx].ptr(); }
        LYNIPV_CXX14_CONSTEXPR reference ref(size_type idx) noexcept { return *ptr(idx); }
        LYNIPV_CXX14_CONSTEXPR const_reference ref(size_type idx) const noexcept { return *ptr(idx); }

        template<class... Args>
        LYNIPV_CXX20_CONSTEXPR reference construct_back(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value) {
            auto& rv = *LYNIPV_CONSTRUCT_AT(ptr(m_size), std::forward<Args>(args)...);
            ++m_size;
            return rv;
        }
        LYNIPV_CXX14_CONSTEXPR void destroy(size_type idx) noexcept { ref(idx).~T(); }

        LYNIPV_CXX14_CONSTEXPR reference operator[](size_type idx) noexcept { return ref(idx); }
        constexpr const_reference operator[](size_type idx) const noexcept { return ref(idx); }

        constexpr size_type size() const noexcept { return m_size; }

        LYNIPV_CXX14_CONSTEXPR size_type inc() noexcept { return ++m_size; }
        LYNIPV_CXX14_CONSTEXPR size_type dec(size_type count = 1) noexcept { return m_size -= count; }
        LYNIPV_CXX14_CONSTEXPR void clear() noexcept(std::is_nothrow_destructible<T>::value) {
            while(m_size) {
                destroy(--m_size);
            }
        }

    private:
        struct alignas(T) inner_storage {
            std::array<unsigned char, sizeof(T)> data;
            pointer ptr() noexcept { return LYNIPV_LAUNDER(reinterpret_cast<pointer>(data.data())); }
            const_pointer ptr() const noexcept { return LYNIPV_LAUNDER(reinterpret_cast<const_pointer>(data.data())); }
        };

        std::array<inner_storage, N> m_data;
        static_assert(sizeof m_data == sizeof(T[N]), "erroneous size");
        size_type m_size = 0;
    };

    template<class T, std::size_t N>
    struct non_trivial_destructor : aligned_storage_non_trivial<T, N> {
        LYNIPV_CXX14_CONSTEXPR non_trivial_destructor() = default;
        LYNIPV_CXX14_CONSTEXPR non_trivial_destructor(const non_trivial_destructor&) = default;
        LYNIPV_CXX14_CONSTEXPR non_trivial_destructor(non_trivial_destructor&&) noexcept = default;
        LYNIPV_CXX14_CONSTEXPR non_trivial_destructor& operator=(const non_trivial_destructor&) = default;
        LYNIPV_CXX14_CONSTEXPR non_trivial_destructor& operator=(non_trivial_destructor&&) noexcept = default;
        LYNIPV_CXX20_CONSTEXPR ~non_trivial_destructor() noexcept(std::is_nothrow_destructible<T>::value) {
            TRACE_ENTER("~non_trival_destructor");
            this->clear();
        }
    };
    template<class T, std::size_t N>
    struct dtor_selector :
        std::conditional<std::is_trivially_destructible<T>::value, aligned_storage_non_trivial<T, N>,
                         non_trivial_destructor<T, N>>::type {};

    template<class T, std::size_t N>
    struct non_trivial_copy_ass : dtor_selector<T, N> {
        LYNIPV_CXX14_CONSTEXPR non_trivial_copy_ass() = default;
        LYNIPV_CXX14_CONSTEXPR non_trivial_copy_ass(const non_trivial_copy_ass&) = default;
        LYNIPV_CXX14_CONSTEXPR non_trivial_copy_ass(non_trivial_copy_ass&&) noexcept = default;
        LYNIPV_CXX14_CONSTEXPR non_trivial_copy_ass& operator=(const non_trivial_copy_ass& other) {
            TRACE_ENTER("operator=(const non_trivial_copy_ass& other)");
            auto& Self = *static_cast<inplace_vector<T, N>*>(this);
            auto& ipo = static_cast<const inplace_vector<T, N>&>(other);
            Self.assign(ipo.begin(), ipo.begin());
            return *this;
        }
        LYNIPV_CXX14_CONSTEXPR non_trivial_copy_ass& operator=(non_trivial_copy_ass&&) noexcept = default;
        LYNIPV_CXX20_CONSTEXPR ~non_trivial_copy_ass() = default;
    };
    template<class T, std::size_t N>
    struct copy_ass_selector :
        std::conditional<trivial_copy_ass<T, N>::value, dtor_selector<T, N>, non_trivial_copy_ass<T, N>>::type {};

    template<class T, std::size_t N>
    struct non_trivial_move_ass : copy_ass_selector<T, N> {
        LYNIPV_CXX14_CONSTEXPR non_trivial_move_ass() = default;
        LYNIPV_CXX14_CONSTEXPR non_trivial_move_ass(const non_trivial_move_ass&) = default;
        LYNIPV_CXX14_CONSTEXPR non_trivial_move_ass(non_trivial_move_ass&&) noexcept = default;
        LYNIPV_CXX14_CONSTEXPR non_trivial_move_ass& operator=(const non_trivial_move_ass&) = default;
        LYNIPV_CXX14_CONSTEXPR non_trivial_move_ass& operator=(non_trivial_move_ass&& other) noexcept(
            std::is_nothrow_move_assignable<T>::value && std::is_nothrow_move_constructible<T>::value) {
            TRACE_ENTER("operator=(non_trivial_move_ass&& other)");
            auto& Self = *static_cast<inplace_vector<T, N>*>(this);
            auto& ipo = static_cast<inplace_vector<T, N>&>(other);
            Self.clear();
            std::move(ipo.begin(), ipo.end(), std::back_inserter(Self));
            ipo.clear();
            return *this;
        }
    };
    template<class T, std::size_t N>
    struct move_ass_selector :
        std::conditional<trivial_move_ass<T, N>::value, copy_ass_selector<T, N>, non_trivial_move_ass<T, N>>::type {};

    template<class T, std::size_t N>
    struct non_trivial_copy_ctor : move_ass_selector<T, N> {
        LYNIPV_CXX14_CONSTEXPR non_trivial_copy_ctor() = default;
        LYNIPV_CXX14_CONSTEXPR non_trivial_copy_ctor(const non_trivial_copy_ctor&) {
            TRACE_ENTER("non_trivial_copy_ctor(const non_trivial_copy_ctor& other)");
        }
        LYNIPV_CXX14_CONSTEXPR non_trivial_copy_ctor(non_trivial_copy_ctor&&) noexcept = default;
        LYNIPV_CXX14_CONSTEXPR non_trivial_copy_ctor& operator=(const non_trivial_copy_ctor& other) = default;
        LYNIPV_CXX14_CONSTEXPR non_trivial_copy_ctor& operator=(non_trivial_copy_ctor&&) noexcept = default;
        LYNIPV_CXX20_CONSTEXPR ~non_trivial_copy_ctor() = default;
    };
    template<class T, std::size_t N>
    struct copy_ctor_selector :
        std::conditional<trivial_copy_ctor<T, N>::value, move_ass_selector<T, N>, non_trivial_copy_ctor<T, N>>::type {};

    template<class T, std::size_t N>
    struct non_trivial_move_ctor : copy_ctor_selector<T, N> {
        LYNIPV_CXX14_CONSTEXPR non_trivial_move_ctor() = default;
        LYNIPV_CXX14_CONSTEXPR non_trivial_move_ctor(const non_trivial_move_ctor&) = default;
        LYNIPV_CXX14_CONSTEXPR non_trivial_move_ctor(non_trivial_move_ctor&& other) noexcept(
            std::is_nothrow_move_constructible<T>::value) {
            TRACE_ENTER("non_trivial_move_ctor(non_trivial_move_ctor&& other)");
            for(decltype(this->size()) idx = 0; idx != other.size(); ++idx) {
                this->construct_back(std::move(other.ref(idx)));
            }
            static_cast<inplace_vector<T, N>&>(other).clear();
        }
        LYNIPV_CXX14_CONSTEXPR non_trivial_move_ctor& operator=(const non_trivial_move_ctor& other) = default;
        LYNIPV_CXX14_CONSTEXPR non_trivial_move_ctor& operator=(non_trivial_move_ctor&&) noexcept = default;
        LYNIPV_CXX20_CONSTEXPR ~non_trivial_move_ctor() = default;
    };
    template<class T, std::size_t N>
    struct move_ctor_selector :
        std::conditional<trivial_move_ctor<T, N>::value, copy_ctor_selector<T, N>, non_trivial_move_ctor<T, N>>::type {};

    template<class T, std::size_t N>
    struct trivial_selector :
        std::conditional<constexpr_compat<T, N>::value, aligned_storage_trivial<T, N>, move_ctor_selector<T, N>>::type {};

    template<class T, std::size_t N>
    struct base_selector : std::conditional<N == 0, aligned_storage_empty<T>, trivial_selector<T, N>>::type {};
} // namespace lyn_inplace_vector_detail

template<class T, std::size_t N>
class inplace_vector : public lyn_inplace_vector_detail::base_selector<T, N> {
    static_assert(std::is_nothrow_destructible<T>::value,
                  "inplace_vector: classes with potentially throwing destructors are prohibited");
    using base = lyn_inplace_vector_detail::base_selector<T, N>;
    using base::construct_back;
    using base::dec;
    using base::destroy;
    using base::ptr;
    using base::ref;

public:
    using base::size;
    using base::operator[];
    using base::clear;

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
    // constructors - copy/move (if any) are defined in the base classes
    constexpr inplace_vector() noexcept = default;

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

    template<
        class InputIt, class U = T,
        typename std::enable_if<std::is_constructible<U, typename std::iterator_traits<InputIt>::value_type>::value, int>::type = 0>
    LYNIPV_CXX14_CONSTEXPR inplace_vector(InputIt first, InputIt last) {
        std::copy(first, last, std::back_inserter(*this));
    }

    template<bool C = std::is_copy_constructible<T>::value, typename std::enable_if<C, int>::type = 0>
    constexpr inplace_vector(std::initializer_list<T> init) : inplace_vector(init.begin(), init.end()) {}

#if __cplusplus >= 202302L && defined(__cpp_lib_containers_ranges)
    template<lyn_inplace_vector_detail::container_compatiblel_range<T> R>
    constexpr inplace_vector(std::from_range_t, R&& rg) {
        if constexpr(std::ranges::sized_range<R>) {
            if(std::ranges::size(rg) > N) throw std::bad_alloc();
            for(auto&& val : rg) unchecked_emplace_back(std::forward<decltype(val)>(val));
        } else {
            for(auto&& val : rg) emplace_back(std::forward<decltype(val)>(val));
        }
    }
#endif

    // assignment - copy/move (if any) are defined in the base classes
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

    template<class InputIt, class U = T>
    LYNIPV_CXX14_CONSTEXPR auto assign(InputIt first, InputIt last) ->
        typename std::enable_if<std::is_constructible<U, typename std::iterator_traits<InputIt>::value_type>::value>::type {
        clear();
        std::copy(first, last, std::back_inserter(*this));
    }

    template<class U = T>
    LYNIPV_CXX14_CONSTEXPR auto assign(std::initializer_list<T> ilist) ->
        typename std::enable_if<std::is_copy_constructible<U>::value>::type {
        if(ilist.size() > capacity()) throw std::bad_alloc();
        clear();
        std::copy(ilist.begin(), ilist.end(), std::back_inserter(*this));
    }

#if __cplusplus >= 202002L
    template<lyn_inplace_vector_detail::container_compatiblel_range<T> R>
    constexpr void assign_range(R&& rg)
        requires std::constructible_from<T&, std::ranges::range_reference_t<R>>
    {
        clear();
        append_range(std::forward<R>(rg));
    }

    template<lyn_inplace_vector_detail::container_compatiblel_range<T> R>
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

    template<lyn_inplace_vector_detail::container_compatiblel_range<T> R>
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
    LYNIPV_CXX14_CONSTEXPR reference front() noexcept { return ref(0); }
    constexpr const_reference front() const noexcept { return ref(0); }
    LYNIPV_CXX14_CONSTEXPR reference back() noexcept { return ref(size() - 1); }
    constexpr const_reference back() const noexcept { return ref(size() - 1); }

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
    template<class U = T>
    LYNIPV_CXX14_CONSTEXPR auto unchecked_resize(size_type count) ->
        typename std::enable_if<std::is_default_constructible<U>::value>::type {
        if(count < size()) {
            shrink_to(count);
        } else {
            while(count != size()) {
                unchecked_emplace_back();
            }
        }
    }

    template<class U = T>
    LYNIPV_CXX14_CONSTEXPR auto unchecked_resize(size_type count, const value_type& value) ->
        typename std::enable_if<std::is_copy_constructible<U>::value>::type {
        if(count < size()) {
            shrink_to(count);
        } else {
            while(count != size()) {
                unchecked_push_back(value);
            }
        }
    }

public:
    template<class U = T>
    LYNIPV_CXX14_CONSTEXPR auto resize(size_type count) -> typename std::enable_if<std::is_default_constructible<U>::value>::type {
        if(count > capacity()) throw std::bad_alloc();
        unchecked_resize(count);
    }

    template<class U = T>
    LYNIPV_CXX14_CONSTEXPR auto resize(size_type count, const value_type& value) ->
        typename std::enable_if<std::is_copy_constructible<U>::value>::type {
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
        // about. Perhaps it can be used for T's with a non-throwing move assignment operator and move constructor.
        // It will at least be ok for trivial types.
    }
    */

public:
    template<class U = T>
    LYNIPV_CXX14_CONSTEXPR auto insert(const_iterator pos, const T& value) ->
        typename std::enable_if<std::is_copy_constructible<U>::value, iterator>::type {
        // static_assert(std::is_nothrow_move_assignable<T>::value, "only nothrow move assignable types may be used for now");
        if(size() == capacity()) throw std::bad_alloc();
        const auto ncpos = const_cast<iterator>(pos);
        unchecked_push_back(value);
        std::rotate(ncpos, std::prev(end()), end());
        return ncpos;
    }

    template<class U = T>
    LYNIPV_CXX14_CONSTEXPR auto insert(const_iterator pos, T&& value) ->
        typename std::enable_if<std::is_move_constructible<U>::value, iterator>::type {
        // static_assert(std::is_nothrow_move_assignable<T>::value, "only nothrow move assignable types may be used for now");
        if(size() == capacity()) throw std::bad_alloc();
        const auto ncpos = const_cast<iterator>(pos);
        unchecked_push_back(std::move(value));
        std::rotate(ncpos, std::prev(end()), end());
        return ncpos;
    }

    template<class U = T>
    LYNIPV_CXX20_CONSTEXPR auto insert(const_iterator pos, size_type count, const T& value) ->
        typename std::enable_if<std::is_copy_constructible<U>::value, iterator>::type {
        // static_assert(std::is_nothrow_move_assignable<T>::value, "only nothrow move assignable types may be used for now");
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
    template<class InputIt, class U = T>
    LYNIPV_CXX20_CONSTEXPR auto insert(const_iterator pos, InputIt first, InputIt last) ->
        typename std::enable_if<std::is_constructible<U, typename std::iterator_traits<InputIt>::value_type>::value &&
                                    !std::is_const<U>::value,
                                iterator>::type {
        // static_assert(std::is_nothrow_move_assignable<T>::value, "only nothrow move assignable types may be used for now");
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
    template<class U = T>
    LYNIPV_CXX14_CONSTEXPR auto insert(const_iterator pos, std::initializer_list<T> ilist) ->
        typename std::enable_if<std::is_copy_constructible<U>::value && !std::is_const<U>::value, iterator>::type {
        return insert(pos, ilist.begin(), ilist.end());
    }

    template<class... Args>
    LYNIPV_CXX14_CONSTEXPR auto emplace(const_iterator pos, Args&&... args) ->
        typename std::enable_if<std::is_constructible<T, Args...>::value, iterator>::type {
        // static_assert(std::is_nothrow_move_assignable<T>::value, "only nothrow move assignable types may be used for now");
        const auto ncpos = const_cast<iterator>(pos);
        emplace_back(std::forward<Args>(args)...);
        std::rotate(ncpos, std::prev(end()), end());
        return ncpos;
    }

    template<class... Args>
    LYNIPV_CXX14_CONSTEXPR auto unchecked_emplace_back(Args&&... args) ->
        typename std::enable_if<std::is_constructible<T, Args...>::value, reference>::type {
        return construct_back(std::forward<Args>(args)...);
    }

    template<class U = T>
    LYNIPV_CXX14_CONSTEXPR auto unchecked_push_back(T const& value) ->
        typename std::enable_if<std::is_copy_constructible<U>::value, reference>::type {
        return construct_back(value);
    }

    template<class U = T>
    LYNIPV_CXX14_CONSTEXPR auto unchecked_push_back(T&& value) ->
        typename std::enable_if<std::is_move_constructible<U>::value, reference>::type {
        return construct_back(std::move(value));
    }

    template<class... Args>
    LYNIPV_CXX14_CONSTEXPR auto emplace_back(Args&&... args) ->
        typename std::enable_if<std::is_constructible<T, Args...>::value, reference>::type {
        if(size() == N) throw std::bad_alloc();
        return unchecked_emplace_back(std::forward<Args>(args)...);
    }

    template<class... Args>
    LYNIPV_CXX14_CONSTEXPR auto try_emplace_back(Args&&... args) ->
        typename std::enable_if<std::is_constructible<T, Args...>::value, pointer>::type {
        if(size() == N) return nullptr;
        return std::addressof(unchecked_emplace_back(std::forward<Args>(args)...));
    }

    template<class U = T>
    LYNIPV_CXX14_CONSTEXPR auto push_back(T const& value) ->
        typename std::enable_if<std::is_copy_constructible<U>::value, reference>::type {
        if(size() == N) throw std::bad_alloc();
        return unchecked_push_back(value);
    }

    template<class U = T>
    LYNIPV_CXX14_CONSTEXPR auto push_back(T&& value) ->
        typename std::enable_if<std::is_move_constructible<U>::value, reference>::type {
        if(size() == N) throw std::bad_alloc();
        return unchecked_push_back(std::move(value));
    }

    template<class U = T>
    LYNIPV_CXX14_CONSTEXPR auto try_push_back(T const& value) ->
        typename std::enable_if<std::is_copy_constructible<U>::value, pointer>::type {
        if(size() == N) return nullptr;
        return std::addressof(unchecked_push_back(value));
    }

    template<class U = T>
    LYNIPV_CXX14_CONSTEXPR auto try_push_back(T&& value) ->
        typename std::enable_if<std::is_move_constructible<U>::value, pointer>::type {
        if(size() == N) return nullptr;
        return std::addressof(unchecked_push_back(std::move(value)));
    }

    LYNIPV_CXX14_CONSTEXPR void pop_back() noexcept { destroy(dec()); }

    template<class U = T>
    LYNIPV_CXX14_CONSTEXPR auto erase(const_iterator first, const_iterator last) ->
        typename std::enable_if<!std::is_const<U>::value, iterator>::type {
        auto ncfirst = const_cast<iterator>(first);
        auto nclast = const_cast<iterator>(last);
        auto removed = static_cast<std::size_t>(std::distance(ncfirst, nclast));
        std::move(nclast, end(), ncfirst);
        for(size_type idx = size() - removed; idx < size(); ++idx) {
            destroy(idx);
        }
        dec(removed);
        return ncfirst;
    }

    template<class U = T>
    LYNIPV_CXX14_CONSTEXPR auto erase(const_iterator pos) -> typename std::enable_if<!std::is_const<U>::value, iterator>::type {
        return erase(pos, std::next(pos));
    }

    template<class U = T>
    LYNIPV_CXX14_CONSTEXPR auto swap(inplace_vector& other) noexcept(N == 0 ||
                                                                     (lyn_inplace_vector_detail::is_nothrow_swappable<T>::value &&
                                                                      std::is_nothrow_move_constructible<T>::value)) ->
        typename std::enable_if<not std::is_const<U>::value>::type {
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
            small.unchecked_push_back(std::move(large[idx]));
        }
        large.shrink_to(small_size);
    }

    LYNIPV_CXX14_CONSTEXPR friend void swap(inplace_vector& lhs, inplace_vector& rhs) noexcept(
        N == 0 || (lyn_inplace_vector_detail::is_nothrow_swappable<T>::value && std::is_nothrow_move_constructible<T>::value)) {
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

template<class T, size_t N, class U = T>
LYNIPV_CXX14_CONSTEXPR typename inplace_vector<T, N>::size_type erase(inplace_vector<T, N>& c, const U& value) {
    auto it = std::remove(c.begin(), c.end(), value);
    auto r = static_cast<typename inplace_vector<T, N>::size_type>(std::distance(it, c.end()));
    c.erase(it, it.end());
    return r;
}

template<class T, size_t N, class Predicate>
LYNIPV_CXX14_CONSTEXPR typename inplace_vector<T, N>::size_type erase_if(inplace_vector<T, N>& c, Predicate pred) {
    auto it = std::remove_if(c.begin(), c.end(), pred);
    auto r = static_cast<typename inplace_vector<T, N>::size_type>(std::distance(it, c.end()));
    c.erase(it, c.end());
    return r;
}

} // namespace lyn

// clean up defines
#undef LYNIPV_CXX14_CONSTEXPR
#undef LYNIPV_CXX20_CONSTEXPR
#undef LYNIPV_CONSTRUCT_AT
#undef LYNIPV_LAUNDER

#endif
