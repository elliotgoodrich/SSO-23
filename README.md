SSO-23
======
SSO-23 is a proof-of-concept string that uses all available bytes for SSO. So when the char type is one byte (char, signed char, unsigned char, etc.) and on a 64-bit computer, this equates to a small-string optimisation capacity of 23 (the last byte is for the null character `'\0'`).

std::string and SSO
-------------------
The most basic layout of a `std::basic_string<CharT>` object is

    struct {
        CharT* ptr;
        size_type size;
        size_type capacity;
    };

where `ptr` is a pointer to a dynamically allocated array of `CharT` of size `capacity`, which has a string of length `size`. However, allocating memory for an empty string (which must terminate with a null character) seems a bit wasteful. Most implementations of `std::basic_string` therefore allow storing a "small string" directly within the string object itself, thereby removing any allocations for small strings. This is known as small (or short) string optimisation.

### Comparison
The table below shows the maximum size string that each library implementation can store inside the `std::basic_string` object directly, along with the sizeof the string object.

|                | std::string  |        | std::u16string |        | std::u32string |        |
|----------------|--------------|--------|----------------|--------|----------------|--------|
| Implementation | SSO capacity | sizeof | SSO capacity   | sizeof | SSO capacity   | sizeof |
| MSVC           | 15           | 32     | 7              | 32     | 3              | 32     |
| GCC            | 15           | 32     | 15             | 48     | 15             | 80     |
| Clang          | 22           | 24     | 10             | 24     | 4              | 24     |
| SSO-23         | 23           | 24     | 11             | 24     | 5              | 24     |

Implementation Details
----------------------

### MSVC 2013
MSVC determines whether we are in SSO by comparing the capacity to the size of the buffer.

    enum { buffer_size = std::max(1, 16 / sizeof(CharT)) };

    union {
        CharT buffer[buffer_size];
        CharT* ptr;
    } m_data;
    size_type m_size;
    size_type m_capacity;

    CharT *data() {
        return (m_capacity > buffer_size) ? m_data.ptr : &m_data.buffer;
    }

### GCC vstring (4.8.1)
As GCC's current `std::basic_string` implementation using COW, we will be looking at their `vstring` class, which is the proposed update. Whether we are using SSO is dependent on whether `m_ptr` points to the local buffer or not. `vstring`'s buffer grows with `sizeof(CharT)`, so the `sizeof(u32vstring)` ends up being 80 bytes.

    enum { local_capacity = 15 };

    CharT* m_ptr;
    size_type m_size;
    union {
        CharT buffer[local_capacity + 1];
        size_type capacity;
    } m_data;

    bool is_local() const {
        return m_ptr() == &m_data.buffer;
    }

    size_type capacity() const {
        return is_local() ? local_capacity : m_data.capacity;
    }

### Clang (rev 223128)
Clang has two different internals to `std::basic_string` controlled by the define `_LIBCPP_ALTERNATE_STRING_LAYOUT`. These differ only on the position of the variables within `long` and `short`. We'll look at the alternate layout as it is closer to the implementation of SSO-23. The least significant bit of capacity is stolen to indicate whether the we are using SSO or not. The (very small) downside to this is that the capacity must be be even. We also show the implementation used for big endian systems.

    enum { short_mask = 0x01 };
    enum { long_mask  = 0x1ul };

    struct long {
        CharT* ptr;
        size_type size;
        size_type capacity;
    };

    enum { buffer_size = std::max(2, (sizeof(long) - 1) / sizeof(CharT)) };

    struct short {
        CharT buffer[buffer_size];
        /* padding type */ padding;
        unsigned char size;
    };

    union {
        long l;
        short s;
    } m_data;

    bool is_long() const {
        return m_data.s.size & short_mask;
    }

    CharT* data() {
        return is_long() ? m_data.l.ptr : &m_data.s.buffer;
    }

    void set_short_size(size_type s) {
        m_data.s.size = (unsigned char)(s << 1);
    }

    size_type get_short_size() const {
        return m_data.s.size >> 1;
    }

    CharT* get_short_pointer() {
        return &m_data.s.buffer;
    }

    size_type capacity() const {
        return (is_long() ? get_long_cap() : buffer_size) - 1;
    }

    size_type get_long_cap() const {
        return m_data.l.capacity & size_type(~long_mask);
    }

    void set_long_cap(size_type s) {
        m_data.l.capacity = long_mask | s;
    }

SSO-23
------
We'll illustrate SSO-23 using a 32-bit system, as an x64 string would take too much horizontal space in the pictures. Also for this we will assume everything is in big endian, instead of the more common little endian, as it is simpler to visually represent. Firstly, we start with the standard representation of a string,

![](images/string1.png?raw=true)

We need to find a representation of this object that does not represent a valid string, so we can use it as a flag to indicate it is using SSO. We know that `size <= capacity`, so if the most significant bit of `size` is 1 but the most significant bit of `capacity` is 0, then we have the incorrect inequality of `size > capacity`. By some bitshifting we move these bits to the end of the string. In doing so, we need to move a bit of `capacity` into the `size` variable. In the pictures we move the second most significant bit, but in practice it doesn't matter.

![](images/string2.png?raw=true)

Now if these last two bits are 1 and 0, then we are using SSO and we use the other bits to hold the string and null character. This gives us a string of length 10 (plus null character). To be able to calculate the string length (keeping in mind that the string can hold null characters), we can use the other 6 bits of the last byte to hold the size (2^6 == 64 > 11).

![](images/sso1.png?raw=true)

To use the last byte as part of the string we must make a few modifications. First, instead of storing the 2 flag bits, we store the binary negation of the bit we got from `size`. So if we are in SSO, the last two bits are both 0. Then, instead of storing the size, we store `11 - size`. 

![](images/sso2.png?raw=true)

When `size == 11`, `11 - size == 0` and the last byte is `0000000 = '\0'`. This allows us to store the full 11 bytes (plus null character) in the string without compromising any functionality.

![](images/sso3.png?raw=true)

In x64 systems, strings of length 23 can be stored in the string object directly.

Applications to current implementations
---------------------------------------
Clang's implementation could easily add another character to its SSO capacity by saving `buffer_size - size` in the `m_data.s.size` so that it becomes the null character when appropriate.

As they both have an additional buffer, both GCC's and MSVC's implementations could be changed to something similar to below, where we store the least significant bit of the last byte as a flag indicating whether or not whether we are in SSO. Then, if we are using SSO, we store `buffer_size - size` in the other bits of this bytes we can use all bytes available for SSO.

    enum { extra_buffer_size = /* Extra capacity >= 1*/ };

    struct long {
        CharT* ptr;
        size_type size;
        size_type capacity;
        Char buffer[extra_buffer_size];
    };

    struct short {
        CharT buffer[sizeof(long) - 1];
        CharT size;
    };

    union {
        long l;
        short s;
    } m_data;

    enum { short_mask = 0x01 };

    bool is_long() const {
        return (unsigned char const*)&m_data.s.size & short_mask;
    }

    void set_short_size(size_type s) {
        m_data.s.size = (unsigned char)(s << 1);
    }

    size_type get_short_size() const {
        return m_data.s.size >> 1;
    }

If these changes were made, and the `extra_buffer_size` is set so that the total sizeof the string is the same, we get this table for SSO capacity.

|                | std::string  |        | std::u16string |        | std::u32string |        |
|----------------|--------------|--------|----------------|--------|----------------|--------|
| Implementation | SSO capacity | sizeof | SSO capacity   | sizeof | SSO capacity   | sizeof |
| MSVC           | 31           | 32     | 15             | 32     | 7              | 32     |
| GCC            | 31           | 32     | 23             | 48     | 19             | 80     |
| Clang          | 23           | 24     | 11             | 24     | 5              | 24     |
| SSO-23         | 23           | 24     | 11             | 24     | 5              | 24     |
