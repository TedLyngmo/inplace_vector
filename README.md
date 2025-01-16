# inplace\_vector
`inplace_vector` for C++11 and above

### Inspired by `std::inplace_vector` from C++26

This is an implementation of
[P0843R14](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p0843r14.html).
Note that before C++26 is released, there may be changes to the specification
and this implementation will try to stay as close as possible to what actually
goes into the final C++26 standard and may therefore change too.

### Status

It is feature complete, however a few member functions taking ranges are only available when compiling with C++20 or newer.
One constructor is only available with C++23 or newer.

|`>=` | member function |
|:---:|:----------------|
|C++23|`template<`_container-compatiblel-range_`<T> R>`<br>`constexpr inplace_vector(std::from_range_t, R&& rg)`|
|C++20|`template<`_container-compatiblel-range_`<T> R>`<br>`constexpr void assign_range(R&& rg)`|
|C++20|`template<`_container-compatiblel-range_`<T> R>`<br>`constexpr void append_range(R&& rg)`|
|C++20|`template<`_container-compatiblel-range_`<T> R>`<br>`constexpr std::ranges::borrowed_iterator_t<R> try_append_range(R&& rg)`|
