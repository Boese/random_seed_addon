#pragma once

#include <node_api.h>
#include <random>

class RandSeed {
private:
    explicit RandSeed();
    ~RandSeed();

    static napi_value New(napi_env env, napi_callback_info info);
    static napi_value Generate(napi_env env, napi_callback_info info);
    static napi_value GenerateSequenceStream(napi_env env, napi_callback_info info);
    static napi_value SetSeed(napi_env env, napi_callback_info info);

    static napi_ref constructor;
    napi_env m_env;
    napi_ref m_wrapper;
    std::unique_ptr<std::mt19937> m_generator;

public:
    static napi_value Init(napi_env env, napi_value exports);
    static void Destructor(napi_env env, void* nativeObject, void* finalize_hint);
};