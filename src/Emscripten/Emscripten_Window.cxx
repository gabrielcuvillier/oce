
#include "Emscripten_Window.hxx"

#if defined(__EMSCRIPTEN__)

#include <Aspect_Convert.hxx>
#include <Aspect_WindowDefinitionError.hxx>

IMPLEMENT_STANDARD_HANDLE (Emscripten_Window, Aspect_Window)
IMPLEMENT_STANDARD_RTTIEXT(Emscripten_Window, Aspect_Window)

// =ls======================================================================
// function : Emscripten_Window
// purpose  :
// =======================================================================
Emscripten_Window::Emscripten_Window (const Handle(Aspect_DisplayConnection)& theDisplay,
                                      const Standard_CString theTitle,
                                      const Standard_Integer thePxWidth,
                                      const Standard_Integer thePxHeight)
: Aspect_Window(),
  myDisplay  (theDisplay),
  myXRight   (thePxWidth),
  myYBottom  (thePxHeight)
{
  int aDummy = 0;
  if (thePxWidth <= 0 || thePxHeight <= 0)
  {
    Aspect_WindowDefinitionError::Raise ("Emscripten_Window, Coordinate(s) out of range");
  }
  //else if (theDisplay.IsNull())
  //{
  //  Aspect_WindowDefinitionError::Raise ("Emscripten_Window, Display connection is undefined");
  //  return;
  //}

  EM_ASM(
    Browser.setCanvasSize(800, 600);
    );
}

// =======================================================================
// function : Destroy
// purpose  :
// =======================================================================
void Emscripten_Window::Destroy()
{
}

// =======================================================================
// function : IsMapped
// purpose  :
// =======================================================================
Standard_Boolean Emscripten_Window::IsMapped() const
{
  return Standard_True;
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
}

// =======================================================================
// function : DoMapping
// purpose  :
// =======================================================================
Standard_Boolean Emscripten_Window::DoMapping() const
{
  return Standard_True; // IsMapped()
}

// =======================================================================
// function : Ratio
// purpose  :
// =======================================================================
Quantity_Ratio Emscripten_Window::Ratio() const
{
  return 4./3.;
}

// =======================================================================
// function : Position
// purpose  :
// =======================================================================
void Emscripten_Window::Position (Standard_Integer& X1, Standard_Integer& Y1,
                          Standard_Integer& X2, Standard_Integer& Y2) const
{
}

// =======================================================================
// function : Size
// purpose  :
// =======================================================================
void Emscripten_Window::Size (Standard_Integer& theWidth,
                      Standard_Integer& theHeight) const
{
}

#endif
