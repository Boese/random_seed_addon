#include "NodeRand.h"
#include "NodeRandStream.h"
#include "napi_extenstions.h"

#include <node_api.h>
#include <assert.h>
#include <random>
#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>

using namespace node_rand;
using namespace napi_extensions;

template<class GENERATOR>
napi_ref NodeRand<GENERATOR>::m_constructor;
template<class GENERATOR>
napi_ref NodeRand<GENERATOR>::m_readableCtor;

template<class GENERATOR>
napi_value NodeRand<GENERATOR>::SetReadable(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[1];
  CheckStatus(napi_get_cb_info(env, info, &argc, args, nullptr, nullptr), env, "Failed to get callback info in SetReadable");
  CheckStatus(napi_create_reference(env, args[0], 1, &m_readableCtor), env, "Failed to create NodeJS Readable reference");
  std::cout << "SetReadable" << __FUNCTION__ << std::endl;
  return nullptr;
}

template<class GENERATOR>
NodeRand<GENERATOR>::NodeRand() : m_env(nullptr), m_wrapper(nullptr), m_seedReset(false), m_GlobalBuffer(), m_generator(std::random_device{}()) {}

template<class GENERATOR>
NodeRand<GENERATOR>::~NodeRand() {
    napi_delete_reference(m_env, m_wrapper);
}

template<class GENERATOR>
void NodeRand<GENERATOR>::Destructor(napi_env env,
                          void* nativeObject,
                          void* /*finalize_hint*/) {
  reinterpret_cast<NodeRand*>(nativeObject)->~NodeRand();
}

template<class GENERATOR>
napi_value NodeRand<GENERATOR>::Init(const std::string& generatorName, napi_env env, napi_value exports) {
  napi_property_descriptor NodeRandProps[] = {
    { "SetSeed", 0, SetSeed, 0, 0, 0, napi_default, 0 },
    { "Generate", 0, Generate, 0, 0, 0, napi_default, 0 },
    { "GenerateSequenceStream", 0, GenerateSequenceStream, 0, 0, 0, napi_default, 0 },
    { "SetReadable", 0, SetReadable, 0, 0, 0, napi_static, 0 }
  };

  // export NodeRand<GENERATOR>
  napi_value cons;
  const std::string className = std::string("NodeRand_" + generatorName);
  CheckStatus(napi_define_class(env, className.c_str(), NAPI_AUTO_LENGTH, New, nullptr, 
    sizeof(NodeRandProps) / sizeof(NodeRandProps[0]), NodeRandProps, &cons), env, "Define class");
  CheckStatus(napi_create_reference(env, cons, 1, &m_constructor), env, "Create class reference");
  CheckStatus(napi_set_named_property(env, exports, className.c_str(), cons), env, "Set ctor property");

  return exports;
}

template<class GENERATOR>
napi_value NodeRand<GENERATOR>::New(napi_env env, napi_callback_info info) {
  assert(m_readableCtor != nullptr && "Must call SetReadable(Readable) before using class!");
  napi_status status;
  napi_value target;
  CheckStatus(napi_get_new_target(env, info, &target), env, "New::get_target");

  if (m_constructor) {
    // Invoked as constructor: `new NodeRand()`
    napi_value jsthis;
    status = napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr);
    assert(status == napi_ok);

    NodeRand* rSeed = new NodeRand<GENERATOR>();

    rSeed->m_env = env;
    status = napi_wrap(env,
                       jsthis,
                       reinterpret_cast<void*>(rSeed),
                       NodeRand::Destructor,
                       nullptr,
                       &rSeed->m_wrapper);
    assert(status == napi_ok);
    
    return jsthis;
  } else {
    std::cout << "Invoked as plain function" << std::endl;
    // Invoked as plain function `NodeRand()`, turn into construct call.
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

template<class GENERATOR>
napi_value NodeRand<GENERATOR>::SetSeed(napi_env env, napi_callback_info info) {
  NodeRand<GENERATOR>* rSeed = GetSelf<NodeRand<GENERATOR>>(env, info);

  // Get seed if specified. If not use std::random_device()
  size_t argc = 1;
  CheckStatus(napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr), env, "SetSeed() get cb info");
  rSeed->m_seedReset = true;

  if (argc == 0) {
    std::cout << "Seed generator with random_device" << std::endl;
    rSeed->m_GlobalBuffer.SetSeed(std::random_device{}());
  } 
  else {
    NapiArgInt64 arg0;
    GetArgs(env, info, arg0);
    int64_t seed = arg0.GetVal();
    std::cout << "Seed generator with set seed: " << seed << std::endl;
    
    rSeed->m_GlobalBuffer.SetSeed(seed);
  }

  return nullptr;
}

template<class GENERATOR>
napi_value NodeRand<GENERATOR>::Generate(napi_env env, napi_callback_info info) {
    NodeRand<GENERATOR>* rSeed = GetSelf<NodeRand<GENERATOR>>(env, info);

    if (rSeed->m_seedReset) {
      auto fakeSeed = rSeed->m_GlobalBuffer.Next();
      std::cout << "Setting fake seed: " << fakeSeed << std::endl;
      rSeed->m_generator.seed(fakeSeed);
      rSeed->m_seedReset = false;
    }

    NapiArgInt64 arg0, arg1;
    GetArgs(env, info, arg0, arg1);

    int64_t min =  napi_extensions::LimitNumberBetweenJavascriptMinMax(arg0.GetVal());
    int64_t max =  napi_extensions::LimitNumberBetweenJavascriptMinMax(arg1.GetVal());

    if (max < min) {
      // TODO: Replace all errors with this call
      std::stringstream ss;
      ss << "Max < Min. Min: " << min << ", Max: " << max << std::endl;
        napi_throw_type_error(env, nullptr, ss.str().c_str());
        return nullptr;
    }
    
    // TODO: Need to grab this from Args
    std::uniform_int_distribution<int64_t> distribution(min, max);

    napi_value result;
    CheckStatus(napi_create_int64(env, distribution(rSeed->m_generator), &result), 
      env, "Failed to create int64");

    return result;
}

// TODO: Change to async function so that push can be called after return
template<class GENERATOR>
napi_value NodeRand<GENERATOR>::GenerateSequenceStream(napi_env env, napi_callback_info info) {
    NodeRand<GENERATOR>* rSeed = GetSelf<NodeRand<GENERATOR>>(env, info);

    NapiArgInt64 arg0, arg1;
    NapiArgUint32 arg2;
    GetArgs(env, info, arg0, arg1, arg2);

    int64_t min = arg0.GetVal();
    int64_t max = arg1.GetVal();
    uint32_t count = arg2.GetVal();

    // get thread-safe seed off global
    int64_t seed = rSeed->m_GlobalBuffer.Next();

    // Return new instance of NodeRandStream
    std::cout << "GenerateSequenceStream seed: " << seed << std::endl;
    GENERATOR g(seed);
    const std::uniform_int_distribution<int64_t> d(min, max);
    return NodeRandStream<int64_t, GENERATOR, std::uniform_int_distribution<int64_t>>::NewInstance(env, rSeed->m_readableCtor, g, d, count);
}

/* Register this as an ES Module */
napi_value Init(napi_env env, napi_value exports) {
  NodeRand<std::mt19937>::Init("mt19937", env, exports);
  NodeRand<std::mt19937_64>::Init("mt19937_64", env, exports);
  std::cout << "done" << std::endl;
  return exports;
}

/* Register this as an ES Module */
NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)