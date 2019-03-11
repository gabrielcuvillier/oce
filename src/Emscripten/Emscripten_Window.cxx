
#include "Emscripten_Window.hxx"

#if defined(__EMSCRIPTEN__)

#include <Aspect_Convert.hxx>
#include <Aspect_WindowDefinitionError.hxx>

#include "emscripten.h"

IMPLEMENT_STANDARD_HANDLE (Emscripten_Window, Aspect_Window)
IMPLEMENT_STANDARD_RTTIEXT(Emscripten_Window, Aspect_Window)

// =ls======================================================================
// function : Emscripten_Window
// purpose  :
// =======================================================================
Emscripten_Window::Emscripten_Window ( )
: Aspect_Window()
{
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
  return Aspect_TOR_UNKNOWN;
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
  int width, height, isFullscreen;
  emscripten_get_canvas_size(&width, &height, &isFullscreen);

  return (Quantity_Ratio)width/(Quantity_Ratio)height;
}

// =======================================================================
// function : Position
// purpose  :
// =======================================================================
void Emscripten_Window::Position (Standard_Integer& X1, Standard_Integer& Y1,
                          Standard_Integer& X2, Standard_Integer& Y2) const
{
  int width, height, isFullscreen;
  emscripten_get_canvas_size(&width, &height, &isFullscreen);
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
  int width, height, isFullscreen;
  emscripten_get_canvas_size(&width, &height, &isFullscreen);
  theWidth = width;
  theHeight = height;
}

#endif
