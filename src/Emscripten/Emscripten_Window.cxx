// Copyright (c) 2019 Gabriel Cuvillier - Continuation Labs
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.

#include <Emscripten_Window.hxx>

#if defined(__EMSCRIPTEN__)

#include <emscripten.h>
#include <emscripten/html5.h>

IMPLEMENT_STANDARD_RTTIEXT(Emscripten_Window, Aspect_Window)

// =======================================================================
// function : Emscripten_Window
// purpose  :
// =======================================================================
Emscripten_Window::Emscripten_Window ( const char* theTargetCanvas )
: Aspect_Window(),
  myTargetCanvas(NULL)
{
  // Do a copy of the input target canvas, as it may come from Emscripten runtime (the lifetime of the pointer is sometime unclear)
  if (theTargetCanvas != NULL) {
    myTargetCanvas = new char [(strlen (theTargetCanvas) + 1 )];
    strcpy (myTargetCanvas,theTargetCanvas);
  }
}

// =======================================================================
// function : Destructor
// purpose  :
// =======================================================================
Emscripten_Window::~Emscripten_Window()
{
  if (myTargetCanvas != NULL) {
    delete [] myTargetCanvas;
    myTargetCanvas = NULL;
  }
}

// =======================================================================
// function : IsMapped
// purpose  :
// =======================================================================
Standard_Boolean Emscripten_Window::IsMapped() const
{
  if (IsVirtual()) {
    return Standard_True;
  }

  EmscriptenVisibilityChangeEvent aVis;
  if (emscripten_get_visibility_status(&aVis) == EMSCRIPTEN_RESULT_SUCCESS) {
    if (aVis.hidden == 0) {
      return Standard_True;
    }
    else {
      return Standard_False;
    }
  }
  else {
    return Standard_False;
  }
}

// =======================================================================
// function : Map
// purpose  :
// =======================================================================
void Emscripten_Window::Map() const
{
}

// =======================================================================
// function : Unmap
// purpose  :
// =======================================================================
void Emscripten_Window::Unmap() const
{
}

// =======================================================================
// function : DoResize
// purpose  :
// =======================================================================
Aspect_TypeOfResize Emscripten_Window::DoResize() const
{
  return Aspect_TOR_UNKNOWN;
}

// =======================================================================
// function : DoMapping
// purpose  :
// =======================================================================
Standard_Boolean Emscripten_Window::DoMapping() const
{
  return IsMapped();
}

// =======================================================================
// function : Ratio
// purpose  :
// =======================================================================
Standard_Real Emscripten_Window::Ratio() const
{
  int width = 1, height = 1;
  emscripten_get_canvas_element_size(myTargetCanvas, &width, &height);

  return (Standard_Real)width/(Standard_Real)height;
}

// =======================================================================
// function : Position
// purpose  :
// =======================================================================
void Emscripten_Window::Position (Standard_Integer& X1, Standard_Integer& Y1,
                          Standard_Integer& X2, Standard_Integer& Y2) const
{
  int width = 0, height = 0;
  emscripten_get_canvas_element_size(myTargetCanvas, &width, &height);
  X1 = 0;
  Y1 = 0;
  X2 = width;
  Y2 = height;
}

// =======================================================================
// function : Size
// purpose  :
// =======================================================================
void Emscripten_Window::Size (Standard_Integer& theWidth,
                              Standard_Integer& theHeight) const
{
  int width = 0, height = 0;
  emscripten_get_canvas_element_size(myTargetCanvas, &width, &height);
  theWidth = width;
  theHeight = height;
}

#endif
