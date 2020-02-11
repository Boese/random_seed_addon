#pragma once

#include <node_api.h>
#include <random>
#include <memory>

namespace rand_addon {

struct GenerateData {
    napi_ref jsthis_ref;
};

class RandSeedStream /* extends Node JS Readable */ {
private:
    std::unique_ptr<std::mt19937> m_generator;
    int64_t m_min;
    int64_t m_max;
    uint32_t m_count;

    struct AsyncGenerateData {
        napi_async_work work;
        napi_threadsafe_function tsfn;
        napi_ref readable_ref;
    };

    RandSeedStream(napi_ref readableCtor, int64_t seed, int64_t min, int64_t max, uint32_t count, napi_value* instance);
    ~RandSeedStream();

    static napi_ref constructor;
    napi_env env_;
    napi_ref wrapper_;

public:
    static void NewInstance(napi_env env, napi_ref readableCtor, int64_t seed, int64_t min, int64_t max, uint32_t count, napi_value* instance);
    static napi_status Init(napi_env env);
    static void Destructor(napi_env env, void* nativeObject, void* finalize_hint);
};

}