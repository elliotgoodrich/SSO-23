SSO-23
======

Small String Optimisation
-------------------------
TODO

Common Implementations
----------------------
TODO

SSO-23
------
SSO-23 is a proof-of-concept string that uses all available bytes for SSO. When the char type has size 1 (char, signed char, unsigned char, etc.) and on x64, this equates to a string of length 23 (plus null char).

We'll illustrate the method using x86 as an x64 string would take too much horizontal space in pictures. Also for this we will assume everything is in big endian, instead of the more common little endian. Firstly, we start with the standard representation of a string

![](images/string1.png?raw=true)

We need to find a value of this string that does not represent a correct string, so we can use it as a flag to indicate that this string is SSO. We know that size <= capacity, so we can save additional_capacity instead of capacity

Where `capacity = size + additional_capacity`. Now note that the most significant bit of size and additional_capacity cannot both be 1, since in this case `size + additional_capacity > std::numeric_limits<std::size_t>::max()`. By some bitshifting we move these bits to the end of the string. In doing so, we need to move a bit of additional_capacity into the size variable. In the pictures we move the second most significant bit.

![](images/string2.png?raw=true)

Now if we read the last two bits as 11, then we are using SSO and we use the other 23 bits to hold the string. This gives us a string of length 11 (+ null char). To be able to calculate the string length (keeping in mind that the string can hold null chars), we can use the other 6 bits of the last byte to hold the size (2^6 = 64 > 11).

![](images/sso1.png?raw=true)

To access the last byte we must make a few modifications. First, instead of storing the 2 flag bits, we store their binary negation. So if we are in SSO, the last 2 bits are 00. Then, instead of storing the size, we store 11 - size. 

![](images/sso2.png?raw=true)

When `size == 11`, 11 - size = 0 and the last byte is 0000000 = '\0'. This allows us to store the full 11 bytes (plus null char) in the string without compromising any functionality.

![](images/sso3.png?raw=true)

In x64 systems, strings of length 23 can be stored in the string object directly.


