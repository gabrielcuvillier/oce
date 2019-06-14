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
#include <Quantity_Ratio.hxx>

class Emscripten_Window : public Aspect_Window
{

public:

  //! Creates an Emscripten window defined by its target canvas id. NULL means the default canvas.
  Standard_EXPORT Emscripten_Window ( const char* theTargetCanvas = NULL );

  //! Destroys the Window and all resourses attached to it
  Standard_EXPORT virtual void Destroy();

  ~Emscripten_Window() {
    Destroy();
  }

  //! Opens the window <me>
  Standard_EXPORT virtual void Map() const;

  //! Closes the window <me>
  Standard_EXPORT virtual void Unmap() const;

  //! Applies the resizing to the window <me>
  Standard_EXPORT virtual Aspect_TypeOfResize DoResize() const;

  //! Apply the mapping change to the window <me>
  Standard_EXPORT virtual Standard_Boolean DoMapping() const;

  //! Returns True if the window <me> is opened
  Standard_EXPORT virtual Standard_Boolean IsMapped() const;

  //! Returns The Window RATIO equal to the physical WIDTH/HEIGHT dimensions
  Standard_EXPORT virtual Quantity_Ratio Ratio() const;

  //! Returns The Window POSITION in PIXEL
  Standard_EXPORT virtual void Position (Standard_Integer& X1,
                                         Standard_Integer& Y1,
                                         Standard_Integer& X2,
                                         Standard_Integer& Y2) const;

  //! Returns The Window SIZE in PIXEL
  Standard_EXPORT virtual void Size (Standard_Integer& theWidth,
                                     Standard_Integer& theHeight) const;

  //! @return native Window handle
  virtual Aspect_Drawable NativeHandle() const {
    return TargetCanvas();  // Return the CanvasTarget
  }

  //! @return parent of native Window handle
  virtual Aspect_Drawable NativeParentHandle() const {
    return 0; // No parent window
  }

  //! @return the Canvas Target Id
  const char* TargetCanvas() const {
    return myTargetCanvas;
  }

protected:

  // Canvas Target Id
  const char* myTargetCanvas;

public:

  DEFINE_STANDARD_RTTI(Emscripten_Window)
};

DEFINE_STANDARD_HANDLE(Emscripten_Window, Aspect_Window)

#endif // __EMSCRIPTEN__
#endif // _Emscripten_Window_H__
