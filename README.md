# inplace\_vector
`inplace_vector` for C++11 and above

### Inspired by `std::inplace_vector` from C++26

This is an implementation of
[P0843R14](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p0843r14.html).
Note that before C++26 is released, there may be changes to the specification
and this implementation will try to stay as close as possible to what actually
goes into the final C++26 standard and may therefore change too.

It's mostly complete except for the member functions that operate on _ranges_.
