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
using namespace napi_extensions;

typedef struct {
  napi_async_work work;
  napi_threadsafe_function tsfn;
  napi_ref readable_ref;
} AddonData;

napi_ref RandSeed::constructor;

RandSeed::RandSeed() : m_generator(std::make_unique<std::mt19937>(std::random_device{}())) {}

RandSeed::~RandSeed() {
    napi_delete_reference(m_env, m_wrapper);
}

void RandSeed::Destructor(napi_env env,
                          void* nativeObject,
                          void* /*finalize_hint*/) {
  reinterpret_cast<RandSeed*>(nativeObject)->~RandSeed();
}

napi_value RandSeed::Init(napi_env env, napi_value exports) {
  napi_property_descriptor properties[] = {
    { "SetSeed", 0, SetSeed, 0, 0, 0, napi_default, 0 },
    { "Generate", 0, Generate, 0, 0, 0, napi_default, 0 },
    { "GenerateSequenceStream", 0, GenerateSequenceStream, 0, 0, 0, napi_default, 0 }
  };
  size_t properties_count = sizeof(properties) / sizeof(properties[0]);

  napi_value cons;
  CheckStatus(napi_define_class(env, "RandSeed", NAPI_AUTO_LENGTH, New, nullptr, properties_count, properties, &cons), env, "Define class");
  CheckStatus(napi_create_reference(env, cons, 1, &constructor), env, "Create class reference");
  CheckStatus(napi_set_named_property(env, exports, "RandSeed", cons), env, "Set ctor property");

  return exports;
}

napi_value RandSeed::New(napi_env env, napi_callback_info info) {
  napi_status status;
  napi_value target;
  CheckStatus(napi_get_new_target(env, info, &target), env, "New::get_target");
  bool is_constructor = target != nullptr;

  if (is_constructor) {
    // Invoked as constructor: `new RandSeed()`
    napi_value jsthis;
    size_t argc = 1;
    napi_value args[1];
    status = napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr);
    assert(status == napi_ok);

    RandSeed* rSeed = new RandSeed();

    rSeed->m_env = env;
    status = napi_wrap(env,
                       jsthis,
                       reinterpret_cast<void*>(rSeed),
                       RandSeed::Destructor,
                       nullptr,
                       &rSeed->m_wrapper);
    assert(status == napi_ok);
    
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
  RandSeed* rSeed = GetSelf<RandSeed>(env, info);

  // Get seed if specified. If not use std::random_device()
  size_t argc = 1;
  CheckStatus(napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr), env, "SetSeed() get cb info");

  if (argc == 0) {
    std::cout << "Seed generator with random_device" << std::endl;
    rSeed->m_generator->seed(std::random_device{}());
  } 
  else {
    std::cout << "Seed generator with seed " << std::endl;
    NapiU32 arg0;
    GetArgs(env, info, arg0);
    rSeed->m_generator->seed(arg0.GetVal());
    std::cout << "Seed: " << arg0.GetVal() << std::endl;
  }

  return nullptr;
}

//TODO: Create arg type checker helper function
napi_value RandSeed::Generate(napi_env env, napi_callback_info info) {
    RandSeed* rSeed = GetSelf<RandSeed>(env, info);

    NapiU32 arg0, arg1;
    GetArgs(env, info, arg0, arg1);
    uint32_t min = arg0.GetVal();
    uint32_t max = arg1.GetVal();

    if (max < min) {
      // TODO: Replace all errors with this call
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
    (void)context; // not used
    AddonData* addon_data = (AddonData*)data;

    napi_value readable_instance;
    napi_status status = napi_get_reference_value(env, addon_data->readable_ref, &readable_instance);
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
}

// TODO: Move these to another cc file. 
// Need to use async_thread_safe_function from execute.
// Code is currently running synchronously in the complete callback
// Move complete Func implementation to a new function thats invoked here from a tsfn
// index.ts test proves this
// NOTE: CANNOT EXECUTE JS IN THIS BLOCK! HAS TO BE DONE FROM TSFN!
void executeFunct(napi_env env, void* data) {
  AddonData* addon_data = (AddonData*)data;

  // We bracket the use of the thread-safe function by this thread by a call to
  // napi_acquire_threadsafe_function() here, and by a call to
  // napi_release_threadsafe_function() immediately prior to thread exit.
  assert(napi_acquire_threadsafe_function(addon_data->tsfn) == napi_ok);

  for (int i = 0; i < 10; i++) {
    // Initiate the call into JavaScript. The call into JavaScript will not
    // have happened when this function returns, but it will be queued.
    assert(napi_call_threadsafe_function(addon_data->tsfn,
                                          addon_data,
                                          napi_tsfn_blocking) == napi_ok);
    std::this_thread::sleep_for (std::chrono::milliseconds(100));
  }
  

  // Indicate that this thread will make no further use of the thread-safe function.
  assert(napi_release_threadsafe_function(addon_data->tsfn,
                                          napi_tsfn_release) == napi_ok);
}

void completeFunc(napi_env env, napi_status status, void* data) {
    AddonData* addon_data = (AddonData*)data;

    napi_value readable_instance;
    status = napi_get_reference_value(env, addon_data->readable_ref, &readable_instance);
    assert(status == napi_ok);

    napi_value push_func;
    status = napi_get_named_property(env, readable_instance, "push", &push_func);
    assert(status == napi_ok);

    // NOTE: MUST CALL NULL WHEN FINISHED OR ERROR WILL OCCUR
    napi_value null_value;
    status = napi_get_null(env, &null_value);
    assert(status == napi_ok);
    status = napi_call_function(env, readable_instance, push_func, 1, &null_value, nullptr);
    assert(status == napi_ok);
    
    uint32_t ref_count;
    status = napi_reference_unref(env, addon_data->readable_ref, &ref_count);
    assert(status == napi_ok);
    assert(ref_count == 0);

    // Clean up the thread-safe function and the work item associated with this
    // run.
    assert(napi_release_threadsafe_function(addon_data->tsfn,
                                            napi_tsfn_release) == napi_ok);
    assert(napi_delete_async_work(env, addon_data->work) == napi_ok);

    // Set values to NULL so JavaScript can order a new run of the thread.
    addon_data->work = NULL;
    addon_data->tsfn = NULL;
    addon_data->readable_ref = NULL;
    delete data;
    data = nullptr;
}

static napi_value _read(napi_env env, napi_callback_info info)
{
  return nullptr;
}

// TODO: Change to async function so that push can be called after return
napi_value RandSeed::GenerateSequenceStream(napi_env env, napi_callback_info info) {
    RandSeed* rSeed = GetSelf<RandSeed>(env, info);

    // Get min/max/size
    size_t argc = 1;
    napi_value args[1];
    napi_value jsthis;
    AddonData* addon_data = new AddonData();
    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr);
    assert(status == napi_ok);

    // create new NodeJS Readable
    napi_value readable_instance;
    napi_value readable_options;
    status = napi_create_object(env, &readable_options);
    assert(status == napi_ok);
    status = napi_new_instance(env, args[0], 1, &readable_options, &readable_instance);
    assert(status == napi_ok);

    // Make sure to implement _read()
    napi_value _readFn;
    status = napi_create_function(env, nullptr, 0, _read, nullptr, &_readFn);
    assert(status == napi_ok);

    status = napi_set_named_property(env, readable_instance, "_read", _readFn);
    assert(status == napi_ok);

    napi_value async_name;
    status = napi_create_string_utf8(env, "generate_async", NAPI_AUTO_LENGTH, &async_name);
    assert(status == napi_ok);

    napi_value tsfn_name;
    status = napi_create_string_utf8(env, "generate_tsfn", NAPI_AUTO_LENGTH, &tsfn_name);
    assert(status == napi_ok);

    status = napi_create_reference(env, readable_instance, 1, &addon_data->readable_ref);
    assert(status == napi_ok);
    
    napi_value push_func;
    status = napi_get_named_property(env, readable_instance, "push", &push_func);
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

    return readable_instance;
}


/* Register this as an ES Module */
napi_value Init(napi_env env, napi_value exports) {
  return RandSeed::Init(env, exports);
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)