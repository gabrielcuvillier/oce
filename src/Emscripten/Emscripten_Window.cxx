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

// cstd
#include <cstring>  // std::strcpy
#include <cmath>    // std::round

// emscripten
#include <emscripten.h>       // EM_ASM, emscripten_get_device_pixel_ratio
#include <emscripten/html5.h> // emscripten_request_animation_frame, emscripten_cancel_animation_frame
                              // emscripten_get_element_css_size, emscripten_get_canvas_element_size,
                              // emscripten_set_canvas_element_size, emscripten_set_resize_callback

IMPLEMENT_STANDARD_RTTIEXT(Emscripten_Window, Aspect_Window)

// =======================================================================
// function : Emscripten_Window
// purpose  :
// =======================================================================
Emscripten_Window::Emscripten_Window ( Standard_CString theTargetCanvas,
                                      std::function<void(void)> theRedrawHandler )
: Aspect_Window(),
  myTargetCanvas(nullptr), // don't initialize it yet, we need to make a copy
  myRedrawHandler(theRedrawHandler),
  myRedrawRequestId(0)
{
  // Do a copy of the input target canvas string, as it may come from Emscripten runtime which has temporary lifetime for the pointer
  if (theTargetCanvas != nullptr) {
    myTargetCanvas = new Standard_Character[ std::strlen(theTargetCanvas) + 1 ];
    std::strcpy(myTargetCanvas, theTargetCanvas);
  }

  // Setup the resize callback: resize the canvas internal size to the canvas CSS size adjusted by the devicePixelRatio
  auto resize_cb = [](int eventType, const EmscriptenUiEvent* /*uiEvent*/, void* userData) -> int {
    if (eventType == EMSCRIPTEN_EVENT_RESIZE) {
      const char* canvasId = static_cast<decltype(canvasId)>(userData);
      // Get devicePixelRatio of the window
      double devicePixelRatio = emscripten_get_device_pixel_ratio();
      // Get the CSS dimensions of the canvas
      double cssWidth = 0., cssHeight = 0.;
      if(emscripten_get_element_css_size(canvasId, &cssWidth, &cssHeight) == EMSCRIPTEN_RESULT_SUCCESS) {
        // Get the actual internal dimensions of the cavans
        int internalWidth = 0, internalHeight = 0;
        if (emscripten_get_canvas_element_size(canvasId, &internalWidth, &internalHeight) == EMSCRIPTEN_RESULT_SUCCESS) {
          // Compute the final requested size = round(cssSize * devicePixelRatio)
          int requestedWidth = std::round(cssWidth * devicePixelRatio);
          int requestedHeight = std::round(cssHeight * devicePixelRatio);

          // If there is a difference between actual and requested, resize the internal canvas dimensions
          if (internalWidth != requestedWidth || internalHeight != requestedHeight) {
            auto result = (emscripten_set_canvas_element_size(canvasId, requestedWidth, requestedHeight) == EMSCRIPTEN_RESULT_SUCCESS);
            (void)result;
          }
        }
      }
      return 1;
    } else {
      return 0;
    }
  };

  // Register the resize callback to the Browser window "resize" event
  {
    auto result = (emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, myTargetCanvas, false, resize_cb) == EMSCRIPTEN_RESULT_SUCCESS);
    (void)result;
  }

  // Do a first initial manual resize, for the purpose of setting things up correctly
  {
    auto result = (resize_cb(EMSCRIPTEN_EVENT_RESIZE, nullptr, myTargetCanvas) == 1);
    (void)result;
  }
}

// =======================================================================
// function : Destructor
// purpose  :
// =======================================================================
Emscripten_Window::~Emscripten_Window()
{
  // Unregister the resize callback assigned to the window "resize" event (NB: pass NULL as the callback function)
  {
    auto result = (emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, myTargetCanvas, false, NULL) == EMSCRIPTEN_RESULT_SUCCESS);
    (void)result;
  }

  // Cancel the request animation frame if there is one, to prevent race condition at callback time (Window is deleted)
  if ( myRedrawRequestId ) {
    emscripten_cancel_animation_frame( myRedrawRequestId );
    myRedrawRequestId = 0;
  }

  // Delete the canvas id
  if (myTargetCanvas != nullptr) {
    delete [] myTargetCanvas;
    myTargetCanvas = nullptr;
  }
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
  auto result = (emscripten_get_canvas_element_size(myTargetCanvas, &width, &height) == EMSCRIPTEN_RESULT_SUCCESS);
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
  auto result = (emscripten_get_canvas_element_size(myTargetCanvas, &width, &height) == EMSCRIPTEN_RESULT_SUCCESS);
  (void)result;
  theWidth = width;
  theHeight = height;
}

// =======================================================================
// function : SetTitle
// purpose  :
// =======================================================================
void Emscripten_Window::SetTitle (const TCollection_AsciiString& theTitle) {
  // Use inline javascript there, because there is no emscripten API do directly set the title of a canvas
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
  // There is no such thing as "InvalidateContent" on a Canvas.
  // As a way to redraw the window, call the Browser requestAnimationFrame to schedule a redraw handler call at the
  // most appropriate time

  // Skip if there already have been a redraw request, to prevent unecessary redraws
  if ( !myRedrawRequestId ) {
    // Call emscripten_request_animation frame with the callback function that will do the Redraw, and store the redraw
    // request identifier
    myRedrawRequestId = emscripten_request_animation_frame( []( double /*time*/, void* userData ) -> int {
      Emscripten_Window* pWindow = static_cast<decltype(pWindow)>( userData );
      if ( pWindow ) {
        // Cleanup the redraw request identifier
        pWindow->SetRedrawRequestId( 0 );
        // And do the redraw
        pWindow->CallRedrawHandler();
        return 1;
      } else {
        return 0;
      }
    }, this);
  }
}

// =======================================================================
// function : GetDevicePixelRatio
// purpose  :
// =======================================================================
double Emscripten_Window::GetDevicePixelRatio() const {
  return emscripten_get_device_pixel_ratio();
}

#endif
