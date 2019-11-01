// Copyright (c) 1999-2014 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// Updates by: Copyright (c) 2019, Gabriel Cuvillier - Continuation-Labs
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

#ifndef _Standard_DefineException_HeaderFile
#define _Standard_DefineException_HeaderFile

#include <Standard_Type.hxx>
#include <Standard_Failure.hxx>

//! Defines an exception class \a C1 that inherits an exception class \a C2.
/*! \a C2 must be Standard_Failure or its ancestor.
    The macro defines empty constructor, copy constructor and static methods Raise() and NewInstance().
    Since Standard_Failure implements class manipulated by handle, DEFINE_STANDARD_RTTI macro is also
    added to enable RTTI.

    When using DEFINE_STANDARD_EXCEPTION in your code make sure you also insert a macro
    DEFINE_STANDARD_HANDLE(C1,C2) before it.
*/

// As Exceptions are disabled on Emscripten (-fno-exceptions + DISABLE_EXCEPTION_CATCHING=1) due to being too slow,
// redefine throw/try/catch to specific exception hijacking code
#if defined(__EMSCRIPTEN__)

#include <exception>

class _TerminateWithStandardFailure {
 public:
  // Constructors
  _TerminateWithStandardFailure(Standard_Failure const &);
  _TerminateWithStandardFailure(std::exception const &);
  // Destructor
  // NB: notice the [[noreturn]] attribute on destructor: it will never return by calling std::terminate()
  [[noreturn]] ~_TerminateWithStandardFailure();

  // Discarded defaults
  _TerminateWithStandardFailure(_TerminateWithStandardFailure const &) = delete;
  _TerminateWithStandardFailure(_TerminateWithStandardFailure &&) = delete;
  _TerminateWithStandardFailure & operator = (_TerminateWithStandardFailure const &) = delete;
  _TerminateWithStandardFailure & operator = (_TerminateWithStandardFailure &&) = delete;
};

// 'throw' is redefined use _TerminateWithStandardFailure functionality
// NB: notice the assignment operator at the end. This is intended, to "absorb" the thrown failure object;
#define throw _TerminateWithStandardFailure _absorb_failure =

// 'try' is redefined to do nothing
#define try

// 'catch' is redefined discard the catch expression as well as eliminate the catch block at compile time
// (thanks to C++17 'if constexpr'), while still defining 'anException' so that the block compiles (thanks to
// C++17 if with initialization statement)
#define catch(x) if constexpr(Standard_Failure anException{}; false)

#endif

#define DEFINE_STANDARD_EXCEPTION(C1,C2) \
 \
class C1 : public C2 { \
  void Throw () const Standard_OVERRIDE { throw *this; } \
public: \
  C1() : C2() {} \
  C1(const Standard_CString theMessage) : C2(theMessage) {} \
  static void Raise(const Standard_CString theMessage = "") { \
    Handle(C1) _E = new C1; \
    _E->Reraise(theMessage); \
  } \
  static void Raise(Standard_SStream& theMessage) { \
    Handle(C1) _E = new C1; \
    _E->Reraise (theMessage); \
  } \
  static Handle(C1) NewInstance(const Standard_CString theMessage = "") { return new C1(theMessage); } \
  DEFINE_STANDARD_RTTI_INLINE(C1,C2) \
};

//! Obsolete macro, kept for compatibility with old code
#define IMPLEMENT_STANDARD_EXCEPTION(C1)

#endif
