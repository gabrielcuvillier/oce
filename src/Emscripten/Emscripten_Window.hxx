// Copyright (c) 2019 Gabriel Cuvillier - Continuation Labs
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.

#ifndef _Emscripten_Window_H__
#define _Emscripten_Window_H__

#if defined(__EMSCRIPTEN__)

#include <functional> // std::function

#include <Aspect_Window.hxx>
#include <Aspect_TypeOfResize.hxx>
#include <Standard_CString.hxx>
#include <Standard_Character.hxx>
#include <Standard.hxx>
#include <Standard_DefineHandle.hxx>

class Emscripten_Window : public Aspect_Window
{

public:

  //! Creates an Emscripten window defined by its target canvas id, and an opaque redraw handler function
  Standard_EXPORT Emscripten_Window ( Standard_CString theTargetCanvas,
                                      std::function<void(void)> theRedrawHandler = []() -> void {},
                                      std::function<void(void)> theResizeHandler = []() -> void {});

  Standard_EXPORT virtual ~Emscripten_Window();

  //! Opens the window <me>
  Standard_EXPORT virtual void Map() const Standard_OVERRIDE ;

  //! Closes the window <me>
  Standard_EXPORT virtual void Unmap() const Standard_OVERRIDE ;

  //! Applies the resizing to the window <me>
  Standard_EXPORT virtual Aspect_TypeOfResize DoResize() const Standard_OVERRIDE ;

  //! Apply the mapping change to the window <me>
  Standard_EXPORT virtual Standard_Boolean DoMapping() const Standard_OVERRIDE ;

  //! Returns True if the window <me> is opened
  Standard_EXPORT virtual Standard_Boolean IsMapped() const Standard_OVERRIDE ;

  //! Returns The Window RATIO equal to the physical WIDTH/HEIGHT dimensions
  Standard_EXPORT virtual Standard_Real Ratio() const Standard_OVERRIDE ;

  //! Returns The Window POSITION in PIXEL
  Standard_EXPORT virtual void Position (Standard_Integer& X1,
                                         Standard_Integer& Y1,
                                         Standard_Integer& X2,
                                         Standard_Integer& Y2) const Standard_OVERRIDE ;

  //! Returns The Window SIZE in PIXEL
  Standard_EXPORT virtual void Size (Standard_Integer& theWidth,
                                     Standard_Integer& theHeight) const Standard_OVERRIDE ;

  //! @return native Window handle
  Standard_EXPORT virtual Aspect_Drawable NativeHandle() const Standard_OVERRIDE {
    return TargetCanvas();  // Return the TargetCanvas
  }

  //! @return parent of native Window handle
  Standard_EXPORT virtual Aspect_Drawable NativeParentHandle() const Standard_OVERRIDE {
    return 0; // No parent window
  }

  Standard_EXPORT virtual Aspect_FBConfig NativeFBConfig() const Standard_OVERRIDE {
    return 0; // No FBConfig
  }

  //! Sets window title.
  Standard_EXPORT virtual void SetTitle (const TCollection_AsciiString& theTitle) Standard_OVERRIDE;

  //! Invalidate entire window content, through calling the window invalidate handler
  Standard_EXPORT virtual void InvalidateContent (const Handle(Aspect_DisplayConnection)& theDisp) Standard_OVERRIDE;

  //! Get the device pixel ratio, used to map window pixels to device pixels
  Standard_EXPORT virtual double GetDevicePixelRatio() const Standard_OVERRIDE;

  // Class-local methods

  Standard_EXPORT Standard_CString TargetCanvas() const {
    return myTargetCanvas;
  }

  Standard_EXPORT int RedrawRequestId() const {
    return myRedrawRequestId;
  }

  Standard_EXPORT void SetRedrawRequestId( int theId ) {
    if (myRedrawRequestId) {
      // There is already a request id => what to do ?
    }
    myRedrawRequestId = theId;
  }

  Standard_EXPORT void CallRedrawHandler() const {
    myRedrawHandler();
  }

  Standard_EXPORT void CallResizeHandler() const {
    myResizeHandler();
  }

private:

  // Canvas Target Id
  Standard_Character* myTargetCanvas;

  // Window Redraw handler
  std::function<void(void)> myRedrawHandler;

  // Request Redraw Identifier
  int myRedrawRequestId;

  // Window Resize handler
  std::function<void(void)> myResizeHandler;

public:

  DEFINE_STANDARD_RTTIEXT(Emscripten_Window,Aspect_Window)

};

DEFINE_STANDARD_HANDLE(Emscripten_Window, Aspect_Window)

#endif // __EMSCRIPTEN__
#endif // _Emscripten_Window_H__
