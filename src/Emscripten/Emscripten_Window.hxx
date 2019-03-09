
#ifndef _Emscripten_Window_H__
#define _Emscripten_Window_H__

#if defined(__EMSCRIPTEN__)

#include <Aspect_Window.hxx>

#include <Aspect_DisplayConnection.hxx>
#include <Aspect_FillMethod.hxx>
#include <Aspect_GradientFillMethod.hxx>
#include <Aspect_Handle.hxx>
#include <Aspect_TypeOfResize.hxx>
#include <Standard.hxx>
#include <Standard_DefineHandle.hxx>
#include <Quantity_NameOfColor.hxx>
#include <Quantity_Parameter.hxx>
#include <Quantity_Ratio.hxx>

#include "emscripten.h"

class Aspect_WindowDefinitionError;
class Aspect_WindowError;
class Aspect_Background;
class Quantity_Color;
class Aspect_GradientBackground;

class Emscripten_Window : public Aspect_Window
{

public:

  Standard_EXPORT Emscripten_Window (const Handle(Aspect_DisplayConnection)& theDisplay,
                                     const Standard_CString theTitle,
                                     const Standard_Integer thePxWidth,
                                     const Standard_Integer thePxHeight);

  //! Destroies the Window and all resourses attached to it
  Standard_EXPORT virtual void Destroy();

  ~Emscripten_Window()
  {
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

  //! @return connection to X Display
  Standard_EXPORT const Handle(Aspect_DisplayConnection)& DisplayConnection() const;

  //! @return native Window handle
  virtual Aspect_Drawable NativeHandle() const
  {
    return 1;
  }

  //! @return parent of native Window handle
  virtual Aspect_Drawable NativeParentHandle() const
  {
    return 0;
  }

protected:

  Handle(Aspect_DisplayConnection) myDisplay; //!< Display connection
  Standard_Integer myXRight;   //!< right  position in pixels
  Standard_Integer myYBottom;  //!< bottom position in pixels

public:

  DEFINE_STANDARD_RTTI(Emscripten_Window)

};

DEFINE_STANDARD_HANDLE(Emscripten_Window, Aspect_Window)

#endif
#endif // _Emscripten_Window_H__
