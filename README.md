# LZ77
Another variation of LZ77 compression and decompression implementation in C.

Made for compression of images for embedded software but can be used for any other need (e.g. text compression).

### Things to note:
- you are responsible for providing right size of result buffers,
- worst case compression scenario should add about 1/8 overhead, so *output* buffer size (*output_limit*) should be at least (*input_length* * 9 / 8 + 1) of size,
- if premature end of output buffer is reached (*output_limit*), algorithm keeps running till end of input buffer (*input_length*), without writing *output* - that is to count and return size of required buffer for further check (if decompression was successful and how big buffer to prepare for another run if not),
- you can execute run with NULL *output* buffer and *output_limit* = 0 to find required output buffer size (that will cost computing time),
- best is to know *output* size (*output_limit*) in advance (keep it along with buffer),
- change *LZ77_MATCH_LENGTH_BITS* define to your needs (5 fits well for data arrays of few K bytes in size, 4 could work better for larger data as it gives farther reach for sliding window).
