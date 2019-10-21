// Copyright (c) 2019 Gabriel Cuvillier - Continuation Labs
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.

#ifndef _Emscripten_ProgressIndicator_H__
#define _Emscripten_ProgressIndicator_H__

#if defined(__EMSCRIPTEN__)

#include <Message_ProgressIndicator.hxx>

#include <emscripten/val.h> // emscripten::val

class Emscripten_ProgressIndicator : public Message_ProgressIndicator {
 public:
  Emscripten_ProgressIndicator( emscripten::val theProgressFunc,
                               emscripten::val theCheckCancelFunc,
                               Standard_Boolean theToAllowYield = Standard_False );

  ~Emscripten_ProgressIndicator();

  Standard_Boolean Show( const Standard_Boolean force = Standard_True ) Standard_OVERRIDE;
  Standard_Boolean UserBreak() Standard_OVERRIDE;

 private:
  Standard_Integer myPreviousProgress;
  emscripten::val myProgressFunc;
  emscripten::val myCheckCancelFunc;
  Standard_Boolean myToAllowYield;
};

#endif
#endif
