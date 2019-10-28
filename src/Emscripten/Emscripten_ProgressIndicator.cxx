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

#include <string>      // std::string (used with emscripten::val)

#include <emscripten.h>     // emscripten_sleep
#include <emscripten/val.h> // emscripten::val

Emscripten_ProgressIndicator::Emscripten_ProgressIndicator( emscripten::val theProgressFunc,
                                                            emscripten::val theCheckCancelFunc,
                                                            Standard_Integer theMessageDepth,
                                                            Standard_Boolean theToAllowYield)
: myPreviousProgress(0),
  myLastCheckedCancelProgress(0),
  myProgressFunc(theProgressFunc),
  myCheckCancelFunc(theCheckCancelFunc),
  myMessageDepth(theMessageDepth),
  myToAllowYield(theToAllowYield && emscripten_get_compiler_setting("ASYNCIFY"))
{}

Emscripten_ProgressIndicator::~Emscripten_ProgressIndicator() {}

Standard_Boolean Emscripten_ProgressIndicator::Show( const Standard_Boolean theToForce ) {
  const Standard_Integer aPosition = static_cast<Standard_Integer>(GetPosition() * 100);
  if ( theToForce || aPosition > myPreviousProgress ) {
    myPreviousProgress = aPosition;
    const Standard_Integer aMaxScope = GetNbScopes();
    const Standard_Integer aMinScope = (aMaxScope - myMessageDepth) >= 1 ? (aMaxScope - myMessageDepth) : 1;
    TCollection_AsciiString aMessage;
    Standard_Boolean bFirst = Standard_True;
    for (Standard_Integer i = GetNbScopes(); i >= aMinScope; i--) {
      Handle(TCollection_HAsciiString) aScopeName = GetScope(i).GetName();
      if (aScopeName.IsNull()) {
        continue;
      } else {
        if (!bFirst) {
          aMessage += " / ";
        } else {
          bFirst = Standard_False;
        }
        aMessage += aScopeName->ToCString();
      }
    }
    myProgressFunc(std::string(aMessage.ToCString()), aPosition);
    return Standard_True;
  } else {
    return Standard_False;
  }
}

Standard_Boolean Emscripten_ProgressIndicator::UserBreak( ) {
  // NB: Need to be compiled with ASYNCIFY=1 and RETAIN_COMPILER_SETTINGS=1 for this to work + ideally usage of ASYNCIFY_WHITELIST
  if (myToAllowYield && (myLastCheckedCancelProgress != myPreviousProgress)) {
    myLastCheckedCancelProgress = myPreviousProgress;
    emscripten_sleep(0);
    return myCheckCancelFunc.call<bool>("call");
  } else {
    return Standard_False;
  }
}

#endif
