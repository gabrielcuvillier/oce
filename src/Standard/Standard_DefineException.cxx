// Copyright (c) 2019, Gabriel Cuvillier - Continuation-Labs

#include <Standard_DefineException.hxx>
#include <Standard_Failure.hxx>

#if defined(__EMSCRIPTEN__)

// std
#include <string>       // std::string
#include <exception>    // std::exception
#include <typeinfo>     // typeid

// emscripten
#include <emscripten.h>        // emscripten_log

_TerminateWithStandardFailure::_TerminateWithStandardFailure(Standard_Failure const & next) noexcept :
    myFailureType(next.DynamicType()->Name()),
    myMessage(next.GetMessageString()) { }

_TerminateWithStandardFailure::_TerminateWithStandardFailure(std::exception const & next) noexcept :
    myFailureType(typeid(next).name()),
    myMessage(next.what()) { }

_TerminateWithStandardFailure::~_TerminateWithStandardFailure() noexcept {
  emscripten_log(EM_LOG_CONSOLE|EM_LOG_ERROR, "%s", std::string(myFailureType + ": " + myMessage).c_str() );
  std::terminate();
};

#endif