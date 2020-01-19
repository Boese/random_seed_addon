<h1>NodeJS native addon wrapper around c++ random library
<small>TODO: Convert this to a class using napi_wrap</small>
</h1>

<p>
Goal is to instead of allocating an array of size n, use the NodeJS Streaming API
to write random numbers to a fixed_size buffer back to javascript. This will hopefully
save some memory, not block eventloop. 

Class should be able to generate reproducible random numbers per instance based on seed.
This will wrap c++ random library.
Should be able to create any random number or sequence of type compatible with JS
ex: int8, uint8, int16, uint16, int32, uint32, double, int64 (limit by js MAX NUM), big_int64, big_uint64

NOTE: if seed is not set, random one will be used

New class: RandSeed (Writeable stream)
public:
RandSeed()
void SetSeed(int32_t num) // set seed

template<typename T>
int32_t Generate(min, max) -> send a random number immediately

template<typename T>
void GenerateSequenceAsync(min, max, size) -> sends random numbers to the underlying stream buffer
</p>