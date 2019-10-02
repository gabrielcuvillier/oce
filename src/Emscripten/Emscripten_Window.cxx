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
#include <emscripten/val.h>
#include <cstring>

IMPLEMENT_STANDARD_RTTIEXT(Emscripten_Window, Aspect_Window)

// =======================================================================
// function : Emscripten_Window
// purpose  :
// =======================================================================
Emscripten_Window::Emscripten_Window ( Standard_CString theTargetCanvas,
                                       emscripten::val theWindowInvalidateHandler )
: Aspect_Window(),
  myTargetCanvas(nullptr), // don't initialize it yet, we need to make a copy
  myWindowInvalidateHandler(theWindowInvalidateHandler)
{
  // Do a copy of the input target canvas string, as it may come from Emscripten runtime which has temporary lifetime for the pointer
  if (theTargetCanvas != nullptr) {
    myTargetCanvas = new Standard_Character[ std::strlen(theTargetCanvas) + 1 ];
    std::strcpy(myTargetCanvas, theTargetCanvas);
  }
}

// =======================================================================
// function : Destructor
// purpose  :
// =======================================================================
Emscripten_Window::~Emscripten_Window()
{
  if (myTargetCanvas != nullptr) {
    delete [] myTargetCanvas;
    myTargetCanvas = nullptr;
  }
  myWindowInvalidateHandler = emscripten::val::undefined();
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
    } else {
      return Standard_False;
    }
  } else {
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
  double width = 0., height = 0.;
  EMSCRIPTEN_RESULT hr = emscripten_get_element_css_size(myTargetCanvas, &width, &height); // Use CSS size to compute ratio

  if ((hr == EMSCRIPTEN_RESULT_SUCCESS) && (height != 0.)) {
    return (width / height);
  } else {
    return 1;
  }
}

// =======================================================================
// function : Position
// purpose  :
// =======================================================================
void Emscripten_Window::Position (Standard_Integer& X1, Standard_Integer& Y1,
                          Standard_Integer& X2, Standard_Integer& Y2) const
{
  double width = 0., height = 0.;
  EMSCRIPTEN_RESULT hr = emscripten_get_element_css_size(myTargetCanvas, &width, &height);  // use CSS size for Position
  X1 = 0;
  Y1 = 0;

  if (hr == EMSCRIPTEN_RESULT_SUCCESS) {
    X2 = width;
    Y2 = height;
  } else {
    X2 = 0;
    Y2 = 0;
  }
}

// =======================================================================
// function : Size
// purpose  :
// =======================================================================
void Emscripten_Window::Size (Standard_Integer& theWidth,
                              Standard_Integer& theHeight) const
{
  double width = 0., height = 0.;
  EMSCRIPTEN_RESULT hr = emscripten_get_element_css_size(myTargetCanvas, &width, &height); // use CSS size for Size

  if (hr == EMSCRIPTEN_RESULT_SUCCESS) {
    theWidth = width;
    theHeight = height;
  }
  else {
    theWidth = 0;
    theHeight = 0;
  }
}

// =======================================================================
// function : SetTitle
// purpose  :
// =======================================================================
void Emscripten_Window::SetTitle (const TCollection_AsciiString& theTitle) {
  EM_ASM({
    const canvas = document.querySelector($0);
    if (canvas) {
      canvas.title = $1;
    }
  }, myTargetCanvas, theTitle.ToCString());
}

// =======================================================================
// function : InvalidateContent
// purpose  :
// =======================================================================
void Emscripten_Window::InvalidateContent (const Handle(Aspect_DisplayConnection)& theDisp) {
  (void)theDisp;
  if (myWindowInvalidateHandler != emscripten::val::undefined() &&
      myWindowInvalidateHandler != emscripten::val::null()) {
    myWindowInvalidateHandler();
  }
}

#endif
