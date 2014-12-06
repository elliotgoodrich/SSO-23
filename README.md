SSO-23
======

Small String Optimisation
-------------------------
TODO

Common Implementations
----------------------

### MSVC 2013
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


    CharT* data() {
        return is_long() ? m_data.l.ptr : &m_data.s.buffer;
    }

    bool is_long() const {
        return m_data.s.size & short_mask;
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
SSO-23 is a proof-of-concept string that uses all available bytes for SSO. So when the char type is one byte (char, signed char, unsigned char, etc.) and on a 64-bit computer, this equates to a small-string optimisation capacity of 23 (the last byte is for the null character `'\0'`).

We'll illustrate the method using a 32-bit system, as an x64 string would take too much horizontal space in the pictures. Also for this we will assume everything is in big endian, instead of the more common little endian. Firstly, we start with the standard representation of a string,

![](images/string1.png?raw=true)

We need to find a value of this string that does not represent a correct string, so we can use it as a flag to indicate that this string is SSO. We know that size <= capacity, so we can save additional_capacity instead of capacity. where `capacity = size + additional_capacity`. Now note that the most significant bit of `size` and `additional_capacity` cannot both be 1, since in this case `size + additional_capacity > std::numeric_limits<std::size_t>::max()`. By some bitshifting we move these bits to the end of the string. In doing so, we need to move a bit of `additional_capacity` into the `size` variable. In the pictures we move the second most significant bit, but in practice it doesn't matter.

![](images/string2.png?raw=true)

Now if these last two bits are both 1, then we are using SSO and we use the other bits to hold the string and null character. This gives us a string of length 10 (plus null character). To be able to calculate the string length (keeping in mind that the string can hold null characters), we can use the other 6 bits of the last byte to hold the size (2^6 == 64 > 11).

![](images/sso1.png?raw=true)

To use the last byte as part of the string we must make a few modifications. First, instead of storing the 2 flag bits, we store their binary negation. So if we are in SSO, the last two bits are both 0. Then, instead of storing the size, we store `11 - size`. 

![](images/sso2.png?raw=true)

When `size == 11`, `11 - size == 0` and the last byte is `0000000 = '\0'`. This allows us to store the full 11 bytes (plus null character) in the string without compromising any functionality.

![](images/sso3.png?raw=true)

In x64 systems, strings of length 23 can be stored in the string object directly.