#include "rand_seed.h"
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

napi_ref RandSeed::m_constructor;
napi_ref RandSeed::m_readableCtor;

napi_value RandSeed::SetReadable(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[1];
  CheckStatus(napi_get_cb_info(env, info, &argc, args, nullptr, nullptr), env, "Failed to get callback info in SetReadable");
  CheckStatus(napi_create_reference(env, args[0], 1, &m_readableCtor), env, "Failed to create NodeJS Readable reference");
  std::cout << "SetReadable" << std::endl;
  return nullptr;
}

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
    { "SetReadable", 0, SetReadable, 0, 0, 0, napi_static, 0 },
    { "SetSeed", 0, SetSeed, 0, 0, 0, napi_default, 0 },
    { "Generate", 0, Generate, 0, 0, 0, napi_default, 0 },
    { "GenerateSequenceStream", 0, GenerateSequenceStream, 0, 0, 0, napi_default, 0 }
  };
  size_t properties_count = sizeof(properties) / sizeof(properties[0]);

  napi_value cons;
  CheckStatus(napi_define_class(env, "RandSeed", NAPI_AUTO_LENGTH, New, nullptr, properties_count, properties, &cons), env, "Define class");
  CheckStatus(napi_create_reference(env, cons, 1, &m_constructor), env, "Create class reference");
  CheckStatus(napi_set_named_property(env, exports, "RandSeed", cons), env, "Set ctor property");

  return exports;
}

napi_value RandSeed::New(napi_env env, napi_callback_info info) {
  assert(m_readableCtor != nullptr && "Must call SetReadable(Readable) before using class!");
  napi_status status;
  napi_value target;
  CheckStatus(napi_get_new_target(env, info, &target), env, "New::get_target");
  bool is_constructor = target != nullptr;

  if (m_constructor) {
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
    status = napi_get_reference_value(env, m_constructor, &cons);
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
    auto seed = std::random_device{}();
    rSeed->m_generator->seed(seed);
    RandSeed::GlobalBuffer::SetSeed(seed);
  } 
  else {
    std::cout << "Seed generator with seed " << std::endl;
    NapiArgInt64 arg0;
    GetArgs(env, info, arg0);
    int64_t seed = arg0.GetVal();
    rSeed->m_generator->seed(seed);
    RandSeed::GlobalBuffer::SetSeed(seed);
    std::cout << "Seed: " << seed << std::endl;
  }

  return nullptr;
}

napi_value RandSeed::Generate(napi_env env, napi_callback_info info) {
    RandSeed* rSeed = GetSelf<RandSeed>(env, info);

    NapiArgInt64 arg0, arg1;
    GetArgs(env, info, arg0, arg1);
    int64_t min = arg0.GetVal();
    int64_t max = arg1.GetVal();

    if (max < min) {
      // TODO: Replace all errors with this call
        napi_throw_type_error(env, nullptr, "Max < Min. Min value must be less than Max");
        return nullptr;
    }
    
    std::uniform_int_distribution<int64_t> distribution(min, max);

    napi_value result;
    CheckStatus(napi_create_int64(env, distribution(*rSeed->m_generator), &result), 
      env, "Failed to create int64");

    return result;
}

// TODO: Change to async function so that push can be called after return
napi_value RandSeed::GenerateSequenceStream(napi_env env, napi_callback_info info) {
    RandSeed* rSeed = GetSelf<RandSeed>(env, info);

    NapiArgInt64 arg0, arg1;
    NapiArgUint32 arg2;
    GetArgs(env, info, arg0, arg1, arg2);

    int64_t min = arg0.GetVal();
    int64_t max = arg1.GetVal();
    uint32_t count = arg2.GetVal();

    // get thread-safe seed off global
    int64_t seed = RandSeed::GlobalBuffer::Next();

    // Return new instance of RandSeedStream
    return RandSeedStream::NewInstance(env, rSeed->m_readableCtor, seed, min, max, count);
}

/* Register this as an ES Module */
napi_value Init(napi_env env, napi_value exports) {
  // TODO: Pass exports?
  return RandSeed::Init(env, exports);
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)