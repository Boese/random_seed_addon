#include "rand_seed.h"
#include "napi_extenstions.hpp"
#include <node_api.h>
#include <assert.h>
#include <random>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using namespace rand_addon;

typedef struct {
  napi_async_work work;
  napi_threadsafe_function tsfn;
  napi_ref jsthis_ref;
} AddonData;

napi_ref RandSeed::constructor;

RandSeed::RandSeed() : m_generator(std::make_unique<std::mt19937>(std::random_device{}())) {}

RandSeed::~RandSeed() {
    m_generator.reset();
    napi_delete_reference(m_env, m_wrapper);
}

void RandSeed::Destructor(napi_env env,
                          void* nativeObject,
                          void* /*finalize_hint*/) {
  reinterpret_cast<RandSeed*>(nativeObject)->~RandSeed();
}

// TODO: Put this in helper lib (for future projects). Other generic methods too like Init, New, Destructor
RandSeed* GetSelf(napi_env env, napi_callback_info info) {
    napi_status status;

    napi_value jsthis;
    status = napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr);
    assert(status == napi_ok);

    RandSeed* rSeed;
    status = napi_unwrap(env, jsthis, reinterpret_cast<void**>(&rSeed));

    return rSeed;
}

napi_value RandSeed::_read(napi_env env, napi_callback_info info)
{
  // read() should never be called from user code. No-op
  return nullptr;
}

napi_value RandSeed::Init(napi_env env, napi_value exports) {
  napi_status status;
  napi_property_descriptor properties[] = {
    { "_read", 0, _read, 0, 0, 0, napi_default, 0 },
    { "SetSeed", 0, SetSeed, 0, 0, 0, napi_default, 0 },
    { "Generate", 0, Generate, 0, 0, 0, napi_default, 0 },
    { "GenerateSequenceStream", 0, GenerateSequenceStream, 0, 0, 0, napi_default, 0 }
  };
  size_t properties_count = sizeof(properties) / sizeof(properties[0]);

  napi_value cons;
  status = napi_define_class(
      env, "RandSeed", NAPI_AUTO_LENGTH, New, nullptr, properties_count, properties, &cons);
  assert(status == napi_ok);

  status = napi_create_reference(env, cons, 1, &constructor);
  assert(status == napi_ok);

  status = napi_set_named_property(env, exports, "RandSeed", cons);
  assert(status == napi_ok);
  return exports;
}

napi_value RandSeed::New(napi_env env, napi_callback_info info) {
  napi_status status;

  napi_value target;
  status = napi_get_new_target(env, info, &target);
  assert(status == napi_ok);
  bool is_constructor = target != nullptr;

  if (is_constructor) {
    // Invoked as constructor: `new RandSeed()`
    napi_value jsthis;
    size_t argc = 1;
    napi_value args[1];
    status = napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr);
    assert(status == napi_ok);

    // Argument - expect Readable Function
    napi_valuetype super_ctor_arg;
    status = napi_typeof(env, args[0], &super_ctor_arg);
    assert(status == napi_ok);
    assert(super_ctor_arg == napi_function);
    napi_value super_ctor = args[0];

    RandSeed* rSeed = new RandSeed();

    rSeed->m_env = env;
    status = napi_wrap(env,
                       jsthis,
                       reinterpret_cast<void*>(rSeed),
                       RandSeed::Destructor,
                       nullptr,
                       &rSeed->m_wrapper);
    assert(status == napi_ok);

    // ctor
    napi_value ctor;
    status = napi_get_reference_value(env, constructor, &ctor);
    
    // "inherit" from NodeJS Readable
    status = napi_extensions::napi_inherits(env, info, ctor, super_ctor, 0, nullptr);
    assert(status == napi_ok);

    // Check push exists from NodeJS Readable
    bool has = false;
    status = napi_has_named_property(env, jsthis, "push", &has);
    assert(has);
    
    return jsthis;
  } else {
    std::cout << "Invoked as plain function" << std::endl;
    // Invoked as plain function `RandSeed()`, turn into construct call.
    status = napi_get_cb_info(env, info, nullptr, nullptr, nullptr, nullptr);
    assert(status == napi_ok);

    napi_value cons;
    status = napi_get_reference_value(env, constructor, &cons);
    assert(status == napi_ok);

    napi_value instance;
    status = napi_new_instance(env, cons, 0, nullptr, &instance);
    assert(status == napi_ok);

    return instance;
  }
}



napi_value RandSeed::SetSeed(napi_env env, napi_callback_info info) {

  napi_status status;

  RandSeed* rSeed = GetSelf(env, info);

  // Get seed if specified. If not use std::random_device()
  size_t argc = 1;
  napi_value args[1];
  status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  assert(status == napi_ok);

  if (argc > 0) {

    napi_valuetype valuetype0;
    status = napi_typeof(env, args[0], &valuetype0);
    assert(status == napi_ok);

    if (valuetype0 != napi_number) {
        napi_throw_type_error(env, nullptr, "Wrong arguments. Expected number for seed");
        return nullptr;
    }

    int seed;
    status = napi_get_value_int32(env, args[0], &seed);
    assert(status == napi_ok);

    std::cout << "Seed generator with seed: " << seed << std::endl;
    rSeed->m_generator->seed(seed);
  } 
  else {
    std::cout << "Seed generator with random_device" << std::endl;
    rSeed->m_generator->seed(std::random_device{}());
  }

  return nullptr;
}

//TODO: Create arg type checker helper function
napi_value RandSeed::Generate(napi_env env, napi_callback_info info) {
    RandSeed* rSeed = GetSelf(env, info);

    // Get min/max
    size_t argc = 2;
    napi_value args[2];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    assert(status == napi_ok);

    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "Wrong number of arguments");
        return nullptr;
    }

    napi_valuetype valuetype0;
    status = napi_typeof(env, args[0], &valuetype0);
    assert(status == napi_ok);

    napi_valuetype valuetype1;
    status = napi_typeof(env, args[1], &valuetype1);
    assert(status == napi_ok);

    if (valuetype0 != napi_number || valuetype1 != napi_number) {
        napi_throw_type_error(env, nullptr, "Wrong arguments");
        return nullptr;
    }

    int32_t min;
    status = napi_get_value_int32(env, args[0], &min);
    assert(status == napi_ok);

    int32_t max;
    status = napi_get_value_int32(env, args[1], &max);
    assert(status == napi_ok);

    if (max < min) {
        napi_throw_type_error(env, nullptr, "Max < Min. Min value must be less than Max");
        return nullptr;
    }

    // Generate random number
    std::uniform_int_distribution<int> distribution(min, max);

    napi_value result;
    assert(napi_create_int32(env,
                        distribution(*(rSeed->m_generator)),
                        &result) == napi_ok);

    // Return the JavaScript integer back to JavaScript.
    return result;
}

static void CallJs(napi_env env, napi_value js_cb, void* context, void* data) {
  std::cout << "CallJs" << std::endl;
    AddonData* addon_data = (AddonData*)data;

    napi_ref jsthisref = (napi_ref)data;
    napi_value jsthis;
    napi_status status = napi_get_reference_value(env, jsthisref, &jsthis);
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

    status = napi_call_function(env, jsthis, js_cb, 1, &res2, nullptr);
    assert(status == napi_ok);
}

// TODO: Move these to another cc file. 
// Need to use async_thread_safe_function from execute.
// Code is currently running synchronously in the complete callback
// Move complete Func implementation to a new function thats invoked here from a tsfn
// index.ts test proves this
// NOTE: CANNOT EXECUTE JS IN THIS BLOCK! HAS TO BE DONE FROM TSFN!
void executeFunct(napi_env env, void* data) {
  std::cout << "executeFunc" << std::endl;
  AddonData* addon_data = (AddonData*)data;

  // We bracket the use of the thread-safe function by this thread by a call to
  // napi_acquire_threadsafe_function() here, and by a call to
  // napi_release_threadsafe_function() immediately prior to thread exit.
  assert(napi_acquire_threadsafe_function(addon_data->tsfn) == napi_ok);

  for (int i = 0; i < 10; i++) {
    // Initiate the call into JavaScript. The call into JavaScript will not
    // have happened when this function returns, but it will be queued.
    assert(napi_call_threadsafe_function(addon_data->tsfn,
                                          addon_data->jsthis_ref,
                                          napi_tsfn_blocking) == napi_ok);
    std::this_thread::sleep_for (std::chrono::milliseconds(100));
  }
  

  // Indicate that this thread will make no further use of the thread-safe function.
  assert(napi_release_threadsafe_function(addon_data->tsfn,
                                          napi_tsfn_release) == napi_ok);

  std::cout << "executeFunc done" << std::endl;
}

void completeFunc(napi_env env, napi_status status, void* data) {
  std::cout << "compleFunc" << std::endl;
    AddonData* addon_data = (AddonData*)data;

    napi_value jsthis;
    status = napi_get_reference_value(env, addon_data->jsthis_ref, &jsthis);
    assert(status == napi_ok);

    napi_value push_func;
    status = napi_get_named_property(env, jsthis, "push", &push_func);
    assert(status == napi_ok);

    // NOTE: MUST CALL NULL WHEN FINISHED OR ERROR WILL OCCUR
    std::cout << "call push with null" << std::endl;
    napi_value null_value;
    status = napi_get_null(env, &null_value);
    assert(status == napi_ok);
    status = napi_call_function(env, jsthis, push_func, 1, &null_value, nullptr);
    assert(status == napi_ok);
    
    uint32_t ref_count;
    status = napi_reference_unref(env, addon_data->jsthis_ref, &ref_count);
    assert(status == napi_ok);
    assert(ref_count == 0);
    //free(data);
    std::cout << "CallJs done" << std::endl;

    // Clean up the thread-safe function and the work item associated with this
    // run.
    assert(napi_release_threadsafe_function(addon_data->tsfn,
                                            napi_tsfn_release) == napi_ok);
    assert(napi_delete_async_work(env, addon_data->work) == napi_ok);

    // Set both values to NULL so JavaScript can order a new run of the thread.
    addon_data->work = NULL;
    addon_data->tsfn = NULL;
    addon_data->jsthis_ref = NULL;
    std::cout << "completeFunc done" << std::endl;
}

// TODO: Change to async function so that push can be called after return
napi_value RandSeed::GenerateSequenceStream(napi_env env, napi_callback_info info) {
    RandSeed* rSeed = GetSelf(env, info);

    // Get min/max/size
    size_t argc = 3;
    napi_value args[3];
    napi_value jsthis;
    AddonData* addon_data = new AddonData();
    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr);
    assert(status == napi_ok);

    // Ensure that no work is currently in progress.
    assert(addon_data->work == NULL && "Only one work item must exist at a time");

    napi_value async_name;
    status = napi_create_string_utf8(env, "generate_async", NAPI_AUTO_LENGTH, &async_name);
    assert(status == napi_ok);

    napi_value tsfn_name;
    status = napi_create_string_utf8(env, "generate_tsfn", NAPI_AUTO_LENGTH, &tsfn_name);
    assert(status == napi_ok);

    status = napi_create_reference(env, jsthis, 1, &addon_data->jsthis_ref);
    assert(status == napi_ok);
    
    napi_value push_func;
    status = napi_get_named_property(env, jsthis, "push", &push_func);
    assert(status == napi_ok);

    // Create thread safe function
    status = napi_create_threadsafe_function(env, push_func, nullptr, tsfn_name, 0, 1, nullptr, nullptr, nullptr, CallJs, &(addon_data->tsfn));
    assert(status == napi_ok);

    // Create async function
    status = napi_create_async_work(env, nullptr, async_name, &executeFunct, &completeFunc, addon_data, &(addon_data->work));
    assert(status == napi_ok);

    // Queue async function
    status = napi_queue_async_work(env, addon_data->work);
    assert(status == napi_ok);

    return nullptr;
}


/* Register this as an ES Module */
napi_value Init(napi_env env, napi_value exports) {
  return RandSeed::Init(env, exports);
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)