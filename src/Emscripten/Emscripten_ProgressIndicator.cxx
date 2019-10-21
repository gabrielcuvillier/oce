// Copyright (c) 2019 Gabriel Cuvillier - Continuation Labs
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.

#include <Emscripten_ProgressIndicator.hxx>

#if defined(__EMSCRIPTEN__)

#include <emscripten.h>     // emscripten_sleep
#include <emscripten/val.h> // emscripten::val

Emscripten_ProgressIndicator::Emscripten_ProgressIndicator( emscripten::val theProgressFunc,
                                                            emscripten::val theCheckCancelFunc,
                                                            Standard_Boolean theToAllowYield)
: myPreviousProgress(0),
  myProgressFunc(theProgressFunc),
  myCheckCancelFunc(theCheckCancelFunc),
  myToAllowYield(theToAllowYield && emscripten_get_compiler_setting("ASYNCIFY")) {

}

Emscripten_ProgressIndicator::~Emscripten_ProgressIndicator() {}

Standard_Boolean Emscripten_ProgressIndicator::Show( const Standard_Boolean /*force = Standard_True*/ ) {
  const Standard_Integer aPosition = (int)GetPosition() * 100;
  if ( aPosition > myPreviousProgress ) {
    myPreviousProgress = aPosition;
    myProgressFunc( myPreviousProgress );
  }
  return Standard_True;
}

Standard_Boolean Emscripten_ProgressIndicator::UserBreak( ) {
  // NB: Need to be compiled with ASYNCIFY=1 and RETAIN_COMPILER_SETTINGS=1 for this to work + ideally usage of ASYNCIFY_WHITELIST
  if (myToAllowYield) {
    emscripten_sleep(0);
    return myCheckCancelFunc.call<bool>("Call");
  } else {
    return Standard_False;
  }
}

#endif
