/*
    napi-api extensions/utility functions.
*/
#pragma once
#include "node_api.h"
#include "assert.h"
#include "mutex"
#include <string>
#include <iostream>
#include <memory>

namespace napi_extensions
{
/*
    Check return status from napi_xxx(). If status != napi_ok, log message along with napi_extended_error_info
*/
inline void CheckStatus(napi_status status, napi_env env, const std::string& message = "unknown")
{
    if (status != napi_ok) {
        printf("[ERROR] %s\n", message.c_str());

        const napi_extended_error_info* info;
        napi_get_last_error_info(env, (const napi_extended_error_info**)&info);
        printf("\tnapi error code: %x\n", info->error_code);
        printf("\tengine error code: %x\n", info->engine_error_code);
        printf("\tmessage: %s\n", info->error_message);
        delete info;

        throw new std::runtime_error(message);
    }
}

/*
    Get and validate napi_callback_info_arguments
*/
// class NapiArgValidator {
// public:
//     virtual bool SetVal(napi_env env, napi_value value) = 0;
// };

class NapiU32 { // : public NapiArgValidator {
private:
    std::shared_ptr<uint32_t> _val;
public:
    NapiU32() : _val(std::make_shared<uint32_t>()) {}

    bool SetVal(napi_env env, napi_value value)
    {
        napi_valuetype type;
        CheckStatus(napi_typeof(env, value, &type), env, "Failed to get napi typeof");
        if (type != napi_number) {
            return false;
        }

        CheckStatus(napi_get_value_uint32(env, value, _val.get()), env, "Failed to get uint32_t value");
        return true;
    }

    uint32_t GetVal()
    {
        return *_val;
    }
};

// TODO: Hide these templates
template<typename T>
void _GetArgs(napi_env env, napi_callback_info info, napi_value* argv, size_t& argv_index, T arg)
{
    arg.SetVal(env, argv[argv_index]);
}

template<typename T, typename ...Args>
void _GetArgs(napi_env env, napi_callback_info info, napi_value* argv, size_t& argv_index, T arg, Args ...args)
{
    arg.SetVal(env, argv[argv_index]);
    _GetArgs(env, info, argv, ++argv_index, args...);
}

template<typename ...Args>
void GetArgs(napi_env env, napi_callback_info info, Args ...args)
{
    const std::size_t n = sizeof...(Args);
    size_t argc = n;

    // TODO: Change to std::array
    napi_value* argv = new napi_value[argc];
    CheckStatus(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr), env, "Failed to get cb info. napi_extensions::GetArgs");
    assert(argc == n && "invalid number of arguments");
    
    size_t index = 0;
    _GetArgs(env, info, argv, index, args...);
    delete [] argv;
}

/*
    Unwrap jsthis into class instance
*/
template<class T> T* GetSelf(napi_env env, napi_callback_info info) {
    napi_value jsthis;
    CheckStatus(napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr), env, "Failed to get jsthis in napi_extensions::GetSelf");
    T* self = nullptr;
    CheckStatus(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&self)), env, "Failed to unwrap in napi_extensions::GetSelf");
    return self;
}

/*
@function - napi_inherts
@description - This function will prototype chain the child class to the parent class. 
    It will also call the parent class super constructor() with args.
@returns - napi_status indicated if it suceeded.

example to inherit from NodeJS Readable:

// Argument - expect Readable Function
napi_value super_ctor = args[0]; <<-- new Addon(Readable)

// ctor
napi_value ctor; <<-- get ctor of addon self: Addon.h: static napi_ref constructor
status = napi_get_reference_value(env, constructor, &ctor);

// "inherit" from NodeJS Readable
status = napi_extensions::napi_inherits(env, info, ctor, super_ctor, 0, nullptr);
assert(status == napi_ok);

*/
static napi_status napi_inherits(napi_env env,           // Node-api env
                        napi_callback_info info,  // Node-api callback info
                        napi_value ctor,          // NodeJS child Function
                        napi_value superCtor,     // NodeJS parent Function
                        size_t superArgc,         // Size of args to call parent super()
                        napi_value* superArgv)     // Args to call parent super()
{
    napi_status status;
    napi_value jsthis;

    // Get JsThis
    status = napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr);
    assert(status == napi_ok);
    if (!status == napi_ok) { return status; }

    // Grap the prototypes of child/parent
    napi_value global, global_object, set_prototype_of, ctor_proto_prop, super_instance;

    status = napi_get_global(env, &global);
    if (status != napi_ok) { return status; }
    status = napi_get_named_property(env, global, "Object", &global_object); // Object
    if (status != napi_ok) { return status; }
    status = napi_get_named_property(env, global_object, "setPrototypeOf", &set_prototype_of); // Object.setPrototypeOf
    if (status != napi_ok) { return status; }
    status = napi_get_named_property(env, ctor, "prototype", &ctor_proto_prop); // ctor.prototype
    if (status != napi_ok) { return status; }

    // New instance of superCtor
    status = napi_new_instance(env, superCtor, superArgc, superArgv, &super_instance);
    if (status != napi_ok) { return status; }

    napi_value argv[2];
    argv[0] = ctor_proto_prop;
    argv[1] = super_instance;

    // Object.setPrototypeOf(ctor.prototype, new superCtor());
    status = napi_call_function(env, global, set_prototype_of, 2, argv, NULL);
    if (status != napi_ok) { return status; }

    // Object.setPrototypeOf(ctor, superCtor);
    argv[0] = ctor;
    argv[1] = superCtor;
    return napi_call_function(env, global, set_prototype_of, 2, argv, NULL);
}

} // end napi_extensions namespace