// Copyright (c) 2019, Gabriel Cuvillier - Continuation-Labs

#include <Standard_DefineException.hxx>
#include <Standard_Failure.hxx>

#if defined(__EMSCRIPTEN__)

// std
#include <exception>    // std::exception, std::terminate
#include <iostream>     // std::cerr
#include <string>       // std::string
#include <typeinfo>     // typeid

_TerminateWithStandardFailure::_TerminateWithStandardFailure(Standard_Failure const & theFailure)
{
  std::cerr << "Exception thrown: " << theFailure.DynamicType()->Name() << ": \"" << theFailure.GetMessageString()
    << "\"" << std::endl;
}

_TerminateWithStandardFailure::_TerminateWithStandardFailure(std::exception const & theStdException)
{
  std::cerr << "Exception thrown: " << typeid(theStdException).name() << ": \"" << theStdException.what()
    << "\"" << std::endl;
}

_TerminateWithStandardFailure::~_TerminateWithStandardFailure() {
  std::terminate();
}


#endif