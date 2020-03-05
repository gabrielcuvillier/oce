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
// Emscripten_Window is only available on Emscripten

// cstd
#include <cstring>  // std::strcpy
#include <cmath>    // std::round

// emscripten
#include <emscripten.h>       // EM_ASM, emscripten_get_device_pixel_ratio
#include <emscripten/html5.h> // emscripten_request_animation_frame, emscripten_cancel_animation_frame
                              // emscripten_get_element_css_size, emscripten_get_canvas_element_size,
                              // emscripten_set_canvas_element_size, emscripten_set_resize_callback, EM_BOOL

IMPLEMENT_STANDARD_RTTIEXT(Emscripten_Window, Aspect_Window)

// =======================================================================
// function : Emscripten_Window
// purpose  :
// =======================================================================
Emscripten_Window::Emscripten_Window ( Standard_CString theTargetCanvas,
                                      std::function<void(void)> theRedrawHandler,
                                      std::function<void(void)> theResizeHandler)
: Aspect_Window(),
  myTargetCanvas(nullptr), // don't initialize it yet, we need to make a copy
  myRedrawHandler(theRedrawHandler),
  myRedrawRequestId(0),
  myResizeHandler(theResizeHandler),
  myLocalDevicePixelRatio(1.),
  myLocalWidth(0),
  myLocalHeight(0),
  myLocalCSSWidth(0.),
  myLocalCSSHeight(0.),
  myIsInit(false)
{
  // Do a copy of the input target canvas string, as it may come from Emscripten runtime which has temporary lifetime
  // for the pointer
  if (theTargetCanvas != nullptr) {
    myTargetCanvas = new Standard_Character[ std::strlen(theTargetCanvas) + 1 ];
    std::strcpy(myTargetCanvas, theTargetCanvas);
  }

  // Setup the Window Resize callback
  auto resize_cb = [](int eventType, const EmscriptenUiEvent* /*uiEvent*/, void* userData) -> EM_BOOL {
    Emscripten_Window* pWindow = static_cast<Emscripten_Window*>( userData );
    if (eventType == EMSCRIPTEN_EVENT_RESIZE && pWindow) {
      pWindow->DoResize();
      return 1;
    } else {
      return 0;
    }
  };
  // Register the window resize callback to the Browser window "resize" event
  EMSCRIPTEN_RESULT result = emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, false, resize_cb);
  (void)result;

  // Do a first initial manual resize, for the purpose of setting things up correctly.
  // Note that we are calling DoResize method in a ctor => but that's OK, as all class fields are properly initialized at
  // this point, no called methods are virtual (except DoResize itself, but we care only about the locally defined one),
  // and we use the "myIsInit" field (that is not yet "true") as a guard to prevent DoResize method call the opaque
  // resize handler provided
  DoResize();

  // Ok, we're done now
  myIsInit = true;
}

// =======================================================================
// function : Destructor
// purpose  :
// =======================================================================
Emscripten_Window::~Emscripten_Window()
{
  // Unregister the resize callback assigned to the window "resize" event (NB: pass NULL as the callback function)
  EMSCRIPTEN_RESULT result = emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, false, NULL);
  (void)result;

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
  // Not implementable
}

// =======================================================================
// function : Unmap
// purpose  :
// =======================================================================
void Emscripten_Window::Unmap() const
{
  // Not implementable
}

// =======================================================================
// function : DoResize
// purpose  :
// =======================================================================
Aspect_TypeOfResize Emscripten_Window::DoResize()
{
  // Resize the canvas internal size to the canvas CSS size adjusted by the devicePixelRatio

  // Get devicePixelRatio of the window, and store the value
  myLocalDevicePixelRatio = emscripten_get_device_pixel_ratio();

  // Get the CSS dimensions of the canvas
  if(emscripten_get_element_css_size(myTargetCanvas, &myLocalCSSWidth, &myLocalCSSHeight) == EMSCRIPTEN_RESULT_SUCCESS) {
    // Compute the requested dimensions = round(cssSize * devicePixelRatio)
    const int requestedWidth = std::round(myLocalCSSWidth * myLocalDevicePixelRatio);
    const int requestedHeight = std::round(myLocalCSSHeight * myLocalDevicePixelRatio);

    // Get the actual internal dimensions of the canvas
    int internalWidth = 0, internalHeight = 0;
    if (emscripten_get_canvas_element_size(myTargetCanvas, &internalWidth, &internalHeight) == EMSCRIPTEN_RESULT_SUCCESS) {
      // If there is a difference between current dimensions and requested dimensions, resize the canvas internal size
      if (internalWidth != requestedWidth || internalHeight != requestedHeight) {
        EMSCRIPTEN_RESULT result = emscripten_set_canvas_element_size(myTargetCanvas, requestedWidth, requestedHeight);
        (void)result;
      }

      // Store the requested dimensions
      myLocalWidth = requestedWidth;
      myLocalHeight = requestedHeight;
    }
  }

  // Call the opaque Resize handler
  // NB: guard against "myIsInit" field not being "true". This is the case when the Window is being constructed, the
  // opaque Resize Handler have not to be called as the Window is not yet attached to anything on the Application side
  if (myIsInit) {
    myResizeHandler();
  }

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
  // Use the CSS size for ratio (and not canvas internal size), in case the canvas might be stretched by CSS
  if (myLocalCSSHeight != 0.) { // protect against height eventually being 0
    return (myLocalCSSWidth / myLocalCSSHeight);
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
  X1 = 0;
  Y1 = 0;
  X2 = myLocalWidth;
  Y2 = myLocalHeight;
}

// =======================================================================
// function : Size
// purpose  :
// =======================================================================
void Emscripten_Window::Size (Standard_Integer& theWidth,
                              Standard_Integer& theHeight) const
{
  theWidth = myLocalWidth;
  theHeight = myLocalHeight;
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
    // Setup the request animation frame callback
    auto request_animation_frame_cb = []( double /*time*/, void* userData ) -> EM_BOOL {
      Emscripten_Window* pWindow = static_cast<Emscripten_Window*>( userData );
      if ( pWindow ) {
        pWindow->DoRedraw();
        return 1;
      } else {
        return 0;
      }
    };

    // Call emscripten_request_animation frame with the callback, and store the redraw request identifier
    myRedrawRequestId = emscripten_request_animation_frame( request_animation_frame_cb , this);
  }
}

// =======================================================================
// function : GetDevicePixelRatio
// purpose  :
// =======================================================================
double Emscripten_Window::GetDevicePixelRatio() const {
  return myLocalDevicePixelRatio;
}

// =======================================================================
// function : TargetCanvas
// purpose  :
// =======================================================================
Standard_CString Emscripten_Window::TargetCanvas() const {
  return myTargetCanvas;
}

// =======================================================================
// function : DoRedraw
// purpose  :
// =======================================================================
void Emscripten_Window::DoRedraw() {
  // Cleanup the redraw request identifier. For some reason this have to be done before calling the redraw handler
  // (otherwise there might miss some redraws on corner cases with usage of InvalidateContent)
  myRedrawRequestId = 0;
  
  // Call the redraw handler
  myRedrawHandler();
}

#endif
