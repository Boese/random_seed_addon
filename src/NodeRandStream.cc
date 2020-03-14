#include "NodeRandStream.h"
#include "napi_extenstions.hpp"
#include <node_api.h>
#include <assert.h>
#include <random>
#include <iostream>
#include <algorithm>
#include <string>
#include <thread>
#include <chrono>

using namespace node_rand;
using namespace napi_extensions;

/// \brief 16kb is max buffer size for Node JS Readable stream. 16kb -> 2000 bytes to represent int64_t
static const uint32_t MAX_BUFFER_SIZE = 2000;

void NodeRandStream::ThreadSafeFunctionFinalized(napi_env env, void* finalize_data, void* finalize_hint)
{
    std::cout << "ThreadSafeFunctionFinalized" << std::endl;
}

void NodeRandStream::ExecuteThreadSafeFunction(napi_env env, napi_value js_cb, void* context, void* data)
{
    std::cout << "ExecuteThreadSafeFunction" << std::endl;
    (void)context; // not used
    ThreadSafeFunctionData* tsfn_data = (ThreadSafeFunctionData*)data;

    napi_value readable_instance;
    napi_status status = napi_get_reference_value(env, tsfn_data->readable_ref, &readable_instance);
    assert(status == napi_ok);

    // Create new arraybuffer int64_t
    const size_t buff_size_in_bytes = tsfn_data->buffer->size()*8; // (for int64_t random numbers)
    int64_t* buff = nullptr;

    napi_value res;
    status = napi_create_arraybuffer(env, buff_size_in_bytes, (void**)&buff, &res);
    assert(status == napi_ok);

    for (size_t i = 0; i < tsfn_data->buffer->size(); i++) {
        buff[i] = tsfn_data->buffer->at(i);
    }

    // TODO: Can this work?
    //status = napi_create_buffer_copy(env, int8_buff_size, (void*)(tsfn_data->buffer), 

    // TODO: This will not work. Readable.push expects Buffer | Uint8Array | string. Might need to create a buffer instead or only support unsigned
    napi_value res2;
    status = napi_create_typedarray(env, napi_typedarray_type::napi_uint8_array, buff_size_in_bytes, res, 0, &res2);
    assert(status == napi_ok);

    status = napi_call_function(env, readable_instance, js_cb, 1, &res2, nullptr);
    assert(status == napi_ok);

    if (tsfn_data->final) {
        std::cout << "tsfn final" << std::endl;
        napi_value null_value;
        status = napi_get_null(env, &null_value);
        assert(status == napi_ok);

        status = napi_call_function(env, readable_instance, js_cb, 1, &null_value, nullptr);
        assert(status == napi_ok);

        napi_reference_unref(env, tsfn_data->readable_ref, nullptr);
        tsfn_data->readable_ref = nullptr;
        tsfn_data->buffer.reset(nullptr);

        // Final call
        assert(napi_release_threadsafe_function(tsfn_data->tsfn,
                                          napi_tsfn_abort) == napi_ok);
        tsfn_data->tsfn = nullptr;
    }

    delete tsfn_data;
    tsfn_data=nullptr;
}

// NOTE: CANNOT EXECUTE JS IN THIS BLOCK! HAS TO BE DONE FROM TSFN!
void NodeRandStream::ExecuteAsyncFunction(napi_env env, void* data)
{
    std::cout << "ExecuteAsyncFunction" << std::endl;
    AsyncFunctionData* async_data = (AsyncFunctionData*)data;

    std::mt19937& generator = *async_data->generator;
    std::uniform_int_distribution<int64_t>& distribution = *async_data->distribution;
  
  assert(napi_acquire_threadsafe_function(async_data->tsfn) == napi_ok);

  uint32_t remaining = async_data->count;
  do {
    uint32_t count = std::min(remaining, MAX_BUFFER_SIZE);
    remaining -= count;

    if (count > 0) {

        ThreadSafeFunctionData* tsfn_data = new ThreadSafeFunctionData();
        tsfn_data->readable_ref = async_data->readable_ref;
        tsfn_data->tsfn = async_data->tsfn;
        tsfn_data->final = remaining < 1;
        tsfn_data->buffer = std::unique_ptr<std::vector<int64_t>>(new std::vector<int64_t>(count));

        auto next = [&distribution, &generator] () { return distribution(generator); };
        std::generate(tsfn_data->buffer->begin(), tsfn_data->buffer->end(), next);

        assert(napi_call_threadsafe_function(async_data->tsfn,
                                          (void*)tsfn_data,
                                          napi_tsfn_blocking) == napi_ok);
    }
    
  } while(remaining > 0);
  
  std::cout << "release tsfn" << std::endl;
  assert(napi_release_threadsafe_function(async_data->tsfn,
                                          napi_tsfn_release) == napi_ok);
}

void NodeRandStream::CompleteAsyncFunction(napi_env env, napi_status status, void* data)
{
    AsyncFunctionData* async_data = (AsyncFunctionData*)data;
    std::cout << "CompleteAsyncFunction" << std::endl;

    napi_delete_async_work(env, async_data->work);
    async_data->work = nullptr;

    napi_reference_unref(env, async_data->readable_ref, nullptr);
    async_data->readable_ref = nullptr;

    delete async_data;
    async_data=nullptr;
}


napi_value NodeRandStream::NewInstance(napi_env env, napi_ref readableCtorRef, int64_t seed, int64_t min, int64_t max, uint32_t count) {

    std::cout << "New Instance: seed: " << seed << std::endl;

    // Start async work
    napi_value readableCtor;
    napi_get_reference_value(env, readableCtorRef, &readableCtor);

    // create new NodeJS Readable
    napi_value readable_instance;
    napi_value readable_options;
    napi_status status = napi_create_object(env, &readable_options);
    assert(status == napi_ok);
    status = napi_new_instance(env, readableCtor, 1, &readable_options, &readable_instance);
    assert(status == napi_ok);

    // Make sure to implement _read()
    napi_value _readFn;
    status = napi_create_function(env, nullptr, 0, _read, nullptr, &_readFn);
    assert(status == napi_ok);

    status = napi_set_named_property(env, readable_instance, "_read", _readFn);
    assert(status == napi_ok);

    std::unique_ptr<std::mt19937> generator = std::make_unique<std::mt19937>(std::random_device{}());
    generator->seed(seed);
    AsyncFunctionData* async_data = new AsyncFunctionData();
    async_data->count = count;
    async_data->min = napi_extensions::LimitNumberBetweenJavascriptMinMax(min);
    async_data->max = napi_extensions::LimitNumberBetweenJavascriptMinMax(max);
    async_data->generator = std::move(generator);

    async_data->distribution = std::make_unique<std::uniform_int_distribution<int64_t>>(min, max);

    napi_value async_name;
    status = napi_create_string_utf8(env, "generate_async", NAPI_AUTO_LENGTH, &async_name);
    assert(status == napi_ok);

    napi_value tsfn_name;
    status = napi_create_string_utf8(env, "generate_tsfn", NAPI_AUTO_LENGTH, &tsfn_name);
    assert(status == napi_ok);

    status = napi_create_reference(env, readable_instance, 1, &async_data->readable_ref);
    assert(status == napi_ok);
    
    napi_value push_func;
    status = napi_get_named_property(env, readable_instance, "push", &push_func);
    assert(status == napi_ok);

    // TODO: Put CallJS, executeFunc, completeFunc in NodeRandStream class
    // Create thread safe function
    status = napi_create_threadsafe_function(env, push_func, nullptr, tsfn_name, 0, 1, nullptr, ThreadSafeFunctionFinalized, nullptr, ExecuteThreadSafeFunction, &(async_data->tsfn));
    assert(status == napi_ok);

    // Create async function
    status = napi_create_async_work(env, nullptr, async_name, ExecuteAsyncFunction, CompleteAsyncFunction, async_data, &(async_data->work));
    assert(status == napi_ok);

    // Queue async function
    status = napi_queue_async_work(env, async_data->work);
    assert(status == napi_ok);

    return readable_instance;
}
