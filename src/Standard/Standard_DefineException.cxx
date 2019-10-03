// Copyright (c) 2019, Gabriel Cuvillier - Continuation-Labs

#include <Standard_DefineException.hxx>
#include <Standard_Failure.hxx>

#if defined(__EMSCRIPTEN__)

#include <string>
#include <iostream>
#include <exception>
#include <typeinfo>

_TerminateWithStandardFailure::_TerminateWithStandardFailure(Standard_Failure const & next) noexcept :
    myFailureType(next.DynamicType()->Name()),
    myMessage(next.GetMessageString()) { }

_TerminateWithStandardFailure::_TerminateWithStandardFailure(std::exception const & next) noexcept :
    myFailureType(typeid(next).name()),
    myMessage(next.what()) { }

_TerminateWithStandardFailure::~_TerminateWithStandardFailure() noexcept {
  std::cerr << myFailureType << ": " << myMessage << std::endl;
  std::terminate(); // Ideally, std::set_terminate should be set by the main application
};

#endif