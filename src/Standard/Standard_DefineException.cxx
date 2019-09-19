#include <Standard_DefineException.hxx>
#include <Standard_Failure.hxx>

#if defined(__EMSCRIPTEN__)

#include <iostream>
#include <exception>

_DelayedTerminate::_DelayedTerminate() noexcept : myFailure(nullptr) { }

_DelayedTerminate::~_DelayedTerminate() noexcept {
  if (myFailure) {
    std::cerr << myFailure->DynamicType() << ": " << myFailure->GetMessageString();
    myFailure = nullptr;
  }
  else {
    std::cerr << "Unknown failure" << std::endl;
  }

  std::terminate();
};

Standard_Failure& _DelayedTerminate::operator,(Standard_Failure& theFailure) noexcept {
  std::cout << "HERE" << std::endl;
  this->myFailure = &theFailure;
  return theFailure;
}

#endif