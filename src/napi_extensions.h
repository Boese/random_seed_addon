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
#include <array>
#include <type_traits>
#include <sstream>
#include <vector>

namespace napi_extensions
{
// TODO: Add logging capabilities in debug mode

/*
    Check return status from napi_xxx(). If status != napi_ok, log message along with napi_extended_error_info
*/
inline void CheckStatus(napi_status status, napi_env env, const std::string& message = "unknown")
{
    if (status != napi_ok) {

        const napi_extended_error_info* info;
        napi_get_last_error_info(env, (const napi_extended_error_info**)&info);

        std::stringstream ss;
        ss << "[ERROR]: " << message << "\n";
        ss << "\tnapi error code: " << info->error_code << "\n";
        ss << "\tengine error code: " << info->engine_error_code << "\n";
        ss << "\tmessage: " << info->error_message << "\n";
        delete info;

        napi_throw_type_error(env, nullptr, ss.str().c_str());
    }
}

/*
    Get and validate napi_callback_info_arguments
*/
template<typename T>
class NapiArgValidator {
public:
    virtual void SetVal(napi_env env, napi_value value) = 0;
    virtual T GetVal() = 0;
};

template<typename T>
class NapiArgNumber : public NapiArgValidator<T> {
protected:
    T _val;

    void CheckNumber(napi_env env, napi_value value) {
        napi_valuetype type;
        CheckStatus(napi_typeof(env, value, &type), env, "Failed to get napi typeof");
        assert(type == napi_number && "Argument invalid. Expecting number!");
    }

public:
    T GetVal() override
    {
        return _val;
    }
};

class NapiArgUint32 : public NapiArgNumber<uint32_t> {
public:
    void SetVal(napi_env env, napi_value value) override
    {
        CheckNumber(env, value);
        uint32_t result;
        CheckStatus(napi_get_value_uint32(env, value, &result), env, "Failed to get uint32 value");
        _val = result;
    }
};

class NapiArgInt64 : public NapiArgNumber<int64_t> {
public:
    void SetVal(napi_env env, napi_value value) override
    {
        CheckNumber(env, value);
        int64_t result;
        CheckStatus(napi_get_value_int64(env, value, &result), env, "Failed to get int64 value");
        _val = result;
    }
};

template<typename T>
inline void _GetArgs(napi_env env, napi_callback_info info, napi_value* argv, size_t& argv_index, T& arg)
{
    arg.SetVal(env, argv[argv_index]);
}

template<typename T, typename ...Args>
inline void _GetArgs(napi_env env, napi_callback_info info, napi_value* argv, size_t& argv_index, T& arg, Args& ...args)
{
    arg.SetVal(env, argv[argv_index]);
    _GetArgs(env, info, argv, ++argv_index, args...);
}

template<typename ...Args>
inline void GetArgs(napi_env env, napi_callback_info info, Args& ...args)
{
    const std::size_t n = sizeof...(Args);
    size_t argc = n;
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
template<class T> 
inline T* GetSelf(napi_env env, napi_callback_info info) {
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
inline napi_status napi_inherits(napi_env env,           // Node-api env
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
    if (status != napi_ok) { return status; }

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

static const int64_t JAVASCRIPT_MAX_SAFE_NUMBER = 0x1FFFFFFFFFFFFF; //(2^53 - 1)
static const int64_t JAVASCRIPT_MIN_SAFE_NUMBER = -(JAVASCRIPT_MAX_SAFE_NUMBER);

/// \brief TODO Fill this out
template<class T>
class NapiObjectWrap {
    // reference to contructor for NapiObjectWrap
    static napi_ref m_thisConstructor;
    // passed along to class to get reference to 'this'
    napi_ref m_wrapper;
    
    static napi_value NewAsConstructor(napi_env env, napi_callback_info info) {
        std::cout << "NewAsConstructor" << std::endl;
        napi_value jsthis;
        CheckStatus(napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr), env, "NewAsConstructor::napi_get_cb_info");

        T* t = new T();

        t->m_env = env;
        CheckStatus(napi_wrap(env,
                        jsthis,
                        reinterpret_cast<void*>(t),
                        NapiObjectWrap<T>::Destructor,
                        nullptr,
                        &t->m_wrapper), env, "NewAsConstructor::napi_wrap");
        
        return jsthis;
    }

    static napi_value NewAsFunction(napi_env env, napi_callback_info info) {
        std::cout << "NewAsFunction" << std::endl;
        CheckStatus(napi_get_cb_info(env, info, nullptr, nullptr, nullptr, nullptr), env, "NewAsFunction::napi_get_cb_info");

        napi_value cons, instance;
        CheckStatus(napi_get_reference_value(env, m_thisConstructor, &cons), env, "NewAsFunction::napi_get_reference_value");
        CheckStatus(napi_new_instance(env, cons, 0, nullptr, &instance), env, "NewAsFunction::napi_new_instance");

        return instance;
    }

    /// \brief Instantiate class either using new or function() syntax. New T().
    /// \return this
    static napi_value New(napi_env env, napi_callback_info info) {
        std::cout << "New" << std::endl;
        return m_thisConstructor ? NewAsConstructor(env, info) : NewAsFunction(env, info);
    }

    /// \brief Calls T->~Destructor()
    static void Destructor(napi_env env, void* nativeObject, void* /*finalize_hint*/) {
        std::cout << "Destructor" << std::endl;
        delete reinterpret_cast<T*>(nativeObject);
    }

protected:
    // napi_env
    /// \note Will be set after construction
    napi_env m_env;

    //virtual const std::vector<napi_property_descriptor>& GetClassProps() const = 0;
    //virtual const std::string& GetClassName() const = 0;

public:
    virtual ~NapiObjectWrap() {
        std::cout << "~NapiObjectWrap" << std::endl;
        napi_delete_reference(m_env, m_wrapper);
    }

    /// \brief Module init function
    static napi_value Init(const std::string& className, napi_env env, napi_value exports) {
        std::cout << "Init: " << std::endl;

        auto props = T::GetClassProps();
        auto NodeRandProps = props.data();

        napi_value cons;
        CheckStatus(napi_define_class(env, className.c_str(), NAPI_AUTO_LENGTH, New, nullptr, 
            props.size(), NodeRandProps, &cons), env, "Define class");
        CheckStatus(napi_create_reference(env, cons, 1, &m_thisConstructor), env, "Create class reference");
        CheckStatus(napi_set_named_property(env, exports, className.c_str(), cons), env, "Set ctor property");

        return exports;
    }
};

template<class T>
napi_ref NapiObjectWrap<T>::m_thisConstructor;


} // end napi_extensions namespace
