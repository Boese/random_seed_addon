#pragma once

#include <node_api.h>
#include <random>

namespace rand_addon {

struct GenerateData {
    napi_ref jsthis_ref;
};

class RandSeed {
private:
    explicit RandSeed();
    ~RandSeed();

    // TODO: Take in Readable to ctor. Use later for RandSeedStream class
    static napi_value New(napi_env env, napi_callback_info info);
    static napi_value SetSeed(napi_env env, napi_callback_info info);
    static napi_value Generate(napi_env env, napi_callback_info info);
    static napi_value GenerateSequenceStream(napi_env env, napi_callback_info info);

    // TODO. Create RandSeedStream class
    /*

        This class can create new instances of RandSeedStream
        It will be responsible for implementing a Readable stream per instance
        It will get it's own generator
        This class will hold a global buffer of random_numbers that it will use as seeds to the instances
        OR use for it's own Generate function

        ie Global seed = 10
        Generate 10000 random numbers off global seed into buffer [1,3,5,...]
        synchronous Generate() -> return random number from buffer
        asynchronous GenerateSequence() -> return new RandSeedStream(random number from buffer as seed). implements readable 
    */
    
    static napi_ref constructor;
    napi_env m_env;
    napi_ref m_wrapper;
    std::unique_ptr<std::mt19937> m_generator;

public:
    static napi_value Init(napi_env env, napi_value exports);
    static void Destructor(napi_env env, void* nativeObject, void* finalize_hint);
};

}