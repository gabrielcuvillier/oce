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
  // Not implemented
}

// =======================================================================
// function : Unmap
// purpose  :
// =======================================================================
void Emscripten_Window::Unmap() const
{
  // Not implemented
}

// =======================================================================
// function : DoResize
// purpose  :
// =======================================================================
Aspect_TypeOfResize Emscripten_Window::DoResize() const
{
  // Not implemented
  return Aspect_TOR_UNKNOWN;
}

// =======================================================================
// function : DoMapping
// purpose  :
// =======================================================================
Standard_Boolean Emscripten_Window::DoMapping() const
{
  // Not implemented. Just return the current mapping status
  return IsMapped();
}

// =======================================================================
// function : Ratio
// purpose  :
// =======================================================================
Standard_Real Emscripten_Window::Ratio() const
{
  double width = 0., height = 0.;

  // Use the CSS size for ratio (and not canvas internal size), in case the canvas might be stretched by CSS
  if ((emscripten_get_element_css_size(myTargetCanvas, &width, &height) == EMSCRIPTEN_RESULT_SUCCESS)
      && (height != 0.)) // protect against height eventually being 0
  {
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
  int width = 0, height = 0;
  // Use the canvas internal size, because the values returned by this function might be be used for glViewport
  const bool result = (emscripten_get_canvas_element_size(myTargetCanvas, &width, &height) == EMSCRIPTEN_RESULT_SUCCESS);
  (void)result;
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
  // Use the canvas internal size, because the values returned by this function are expected to be used for glViewport
  // NB: might be buggy if the canvas internal size does not match with the WebGL drawing buffer size
  const bool result = (emscripten_get_canvas_element_size(myTargetCanvas, &width, &height) == EMSCRIPTEN_RESULT_SUCCESS);
  (void)result;
  theWidth = width;
  theHeight = height;
}

// =======================================================================
// function : SetTitle
// purpose  :
// =======================================================================
void Emscripten_Window::SetTitle (const TCollection_AsciiString& theTitle) {
  std::cout << "Emscripten_Window::SetTitle" << std::endl;
  EM_ASM_({
    var canvas = document.querySelector(UTF8ToString($0));
    if (canvas) {
      canvas.title = UTF8ToString($1);
    }
  }, (const char*)(myTargetCanvas), (const char*)(theTitle.ToCString()));
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
