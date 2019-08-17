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

#include <Aspect_Window.hxx>
#include <Aspect_Handle.hxx>
#include <Aspect_TypeOfResize.hxx>
#include <Standard.hxx>
#include <Standard_DefineHandle.hxx>

class Emscripten_Window : public Aspect_Window
{

public:

  //! Creates an Emscripten window defined by its target canvas id. NULL means the default canvas.
  Standard_EXPORT Emscripten_Window ( const char* theTargetCanvas = NULL );

  //! Destroys the Window and all resourses attached to it
  Standard_EXPORT virtual void Destroy();

  Standard_EXPORT ~Emscripten_Window() {
    Destroy();
  }

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

  //! @return the Canvas Target Id
  Standard_EXPORT const char* TargetCanvas() const {
    return myTargetCanvas;
  }

  //! @return native Window handle
  virtual Aspect_Drawable NativeHandle() const Standard_OVERRIDE {
    return TargetCanvas();  // Return the CanvasTarget
  }

  //! @return parent of native Window handle
  virtual Aspect_Drawable NativeParentHandle() const Standard_OVERRIDE {
    return 0; // No parent window
  }

  virtual Aspect_FBConfig NativeFBConfig() const Standard_OVERRIDE {
    return 0;
  }

private:

  // Canvas Target Id
  const char* myTargetCanvas;

public:

  DEFINE_STANDARD_RTTIEXT(Emscripten_Window,Aspect_Window)

};

DEFINE_STANDARD_HANDLE(Emscripten_Window, Aspect_Window)

#endif // __EMSCRIPTEN__
#endif // _Emscripten_Window_H__
