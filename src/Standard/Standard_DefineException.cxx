// Copyright (c) 2019, Gabriel Cuvillier - Continuation-Labs

#include <Standard_DefineException.hxx>
#include <Standard_Failure.hxx>

#if defined(__EMSCRIPTEN__)

// std
#include <exception>    // std::exception, std::terminate
#include <iostream>     // std::cerr
#include <string>       // std::string
#include <typeinfo>     // typeid

_TerminateWithStandardFailure::_TerminateWithStandardFailure(Standard_Failure const & theFailure) noexcept :
    myFailureType(theFailure.DynamicType()->Name()),
    myMessage(theFailure.GetMessageString()) { }

_TerminateWithStandardFailure::_TerminateWithStandardFailure(std::exception const & theStdException) noexcept :
    myFailureType(typeid(theStdException).name()),
    myMessage(theStdException.what()) { }

_TerminateWithStandardFailure::~_TerminateWithStandardFailure() noexcept {
  std::cerr << "Exception thrown: " << myFailureType << ": \"" << myMessage << "\"" << std::endl;
  std::terminate();
};

#endif