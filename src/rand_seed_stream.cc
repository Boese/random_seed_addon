#include "rand_seed_stream.h"
#include "napi_extenstions.hpp"
#include <node_api.h>
#include <assert.h>
#include <random>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using namespace rand_addon;
using namespace napi_extensions;

void RandSeedStream::ThreadSafeFunctionFinalized(napi_env env, void* finalize_data, void* finalize_hint)
{
    std::cout << "ThreadSafeFunctionFinalized" << std::endl;
}

void RandSeedStream::ExecuteThreadSafeFunction(napi_env env, napi_value js_cb, void* context, void* data)
{
    std::cout << "ExecuteThreadSafeFunction" << std::endl;
    (void)context; // not used
    ThreadSafeFunctionData* tsfn_data = (ThreadSafeFunctionData*)data;

    napi_value readable_instance;
    napi_status status = napi_get_reference_value(env, tsfn_data->readable_ref, &readable_instance);
    assert(status == napi_ok);

    // Create new arraybuffer uint32_t[]
    const uint32_t uint32_buff_size = 4000;
    const uint32_t uint8_buff_size = uint32_buff_size*4; // (for uint32_t random numbers)

    uint32_t* buff = nullptr;

    napi_value res;
    status = napi_create_arraybuffer(env, uint8_buff_size, (void**)&buff, &res);
    assert(status == napi_ok);

    for (uint32_t i = 0; i < uint32_buff_size; i++) {
        buff[i] = i;
    }

    napi_value res2;
    status = napi_create_typedarray(env, napi_typedarray_type::napi_uint8_array, uint8_buff_size, res, 0, &res2);
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

        // Final call
        assert(napi_release_threadsafe_function(tsfn_data->tsfn,
                                          napi_tsfn_abort) == napi_ok);
        tsfn_data->tsfn = nullptr;
    }

    delete tsfn_data;
    tsfn_data=nullptr;
}

// NOTE: CANNOT EXECUTE JS IN THIS BLOCK! HAS TO BE DONE FROM TSFN!
void RandSeedStream::ExecuteAsyncFunction(napi_env env, void* data)
{
    std::cout << "ExecuteAsyncFunction" << std::endl;
    AsyncFunctionData* async_data = (AsyncFunctionData*)data;
  
  assert(napi_acquire_threadsafe_function(async_data->tsfn) == napi_ok);

  // TODO: Use actual args/generator for creating random numbers
  for (int i = 0; i < 100; i++) {
      ThreadSafeFunctionData* data = new ThreadSafeFunctionData();
      data->readable_ref = async_data->readable_ref;
      data->tsfn = async_data->tsfn;
      if (i == 99) {
          data->final = true;
      }
    assert(napi_call_threadsafe_function(async_data->tsfn,
                                          (void*)data,
                                          napi_tsfn_blocking) == napi_ok);
  }
  

  std::cout << "release tsfn" << std::endl;
  assert(napi_release_threadsafe_function(async_data->tsfn,
                                          napi_tsfn_release) == napi_ok);
}

void RandSeedStream::CompleteAsyncFunction(napi_env env, napi_status status, void* data)
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


napi_value RandSeedStream::NewInstance(napi_env env, napi_ref readableCtorRef, int64_t seed, int64_t min, int64_t max, uint32_t count) {

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
    async_data->min = min;
    async_data->max = max;
    async_data->generator = std::move(generator);

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

    // TODO: Put CallJS, executeFunc, completeFunc in RandSeedStream class
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
