// Copyright (c) 2019 Gabriel Cuvillier - Continuation Labs
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.

#ifndef _StdPrs_DeferredHLRShapeHandler_HeaderFile
#define _StdPrs_DeferredHLRShapeHandler_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>
#include <Standard_Transient.hxx>

#include <Prs3d_Drawer.hxx>
#include <Prs3d_Presentation.hxx>
#include <Prs3d_Projector.hxx>
#include <TopoDS_Shape.hxx>

class StdPrs_DeferredHLRShapeHandler;
DEFINE_STANDARD_HANDLE(StdPrs_DeferredHLRShapeHandler, Standard_Transient)

class StdPrs_DeferredHLRShapeHandler : public Standard_Transient {
 public:
  StdPrs_DeferredHLRShapeHandler();

  ~StdPrs_DeferredHLRShapeHandler();

  virtual void Add(const Handle(Prs3d_Presentation) &thePresentation,
                   const TopoDS_Shape &theShape,
                   const Handle(Prs3d_Drawer)& theDrawer,
                   const Handle(Prs3d_Projector)& theProjector) = 0;

 private:
  DEFINE_STANDARD_RTTIEXT(StdPrs_DeferredHLRShapeHandler, Standard_Transient)
};

#endif // _StdPrs_DeferredHLRShapeHandler_HeaderFile
