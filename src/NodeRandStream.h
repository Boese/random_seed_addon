#pragma once

#include <node_api.h>
#include <random>
#include <memory>

namespace node_rand {

class NodeRandStream /* extends Node JS Readable */ {
private:
    /// \brief data needed during async function queue
    struct AsyncFunctionData {
        // async work item
        napi_async_work work{nullptr};
        // threadsafe function instance
        napi_threadsafe_function tsfn{nullptr};

        // rng instance
        std::unique_ptr<std::mt19937> generator{nullptr};

        // distribution instance
        std::unique_ptr<std::uniform_int_distribution<int64_t>> distribution{nullptr};

        // min random number
        int64_t min{0};

        // max random number
        int64_t max{0};

        // how many random numbers to generate
        uint32_t count{0};

        // reference to node js Readable
        napi_ref readable_ref{nullptr};
    };

    /// \brief data needed during tsfn function call
    struct ThreadSafeFunctionData {
        // signal this is the last call
        bool final{false};

        // numbers to write to buffer
        std::unique_ptr<std::vector<int64_t>> buffer{nullptr};

        // threadsafe function instance
        napi_threadsafe_function tsfn{nullptr};

        // reference to node js Readable
        napi_ref readable_ref{nullptr};
    };

    // standard uniform int distribution
    

    static void ExecuteThreadSafeFunction(napi_env env, napi_value js_cb, void* context, void* data);
    static void ThreadSafeFunctionFinalized(napi_env env, void* finalize_data, void* finalize_hint);
    static void ExecuteAsyncFunction(napi_env env, void* data);
    static void CompleteAsyncFunction(napi_env env, napi_status status, void* data);

    /// \brief Required to implement _read for Node JS Readable. No-op
    static napi_value _read(napi_env env, napi_callback_info info) { return nullptr; }

public:
    NodeRandStream() = delete;

    /// \brief Instantiate class either using new or function() syntax
    /// \return this
    static napi_value NewInstance(napi_env env, napi_ref readableCtorRef, int64_t seed, int64_t min, int64_t max, uint32_t count);
};

}