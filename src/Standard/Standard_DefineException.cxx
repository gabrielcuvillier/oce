// Copyright (c) 2019, Gabriel Cuvillier - Continuation-Labs

#include <Standard_DefineException.hxx>
#include <Standard_Failure.hxx>

#if defined(__EMSCRIPTEN__)

#include <iostream>
#include <exception>

_TerminateWithStandardFailure::_TerminateWithStandardFailure(Standard_Failure const & next) noexcept :
    myFailureType(next.DynamicType()->Name()), myMessage(next.GetMessageString()) { }

_TerminateWithStandardFailure::~_TerminateWithStandardFailure() noexcept {
  std::cerr << myFailureType << ": " << myMessage << std::endl;
  std::terminate();
};

#endif