// Copyright (c) 2019 Gabriel Cuvillier - Continuation Labs
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.

#include <StdPrs_DeferredHLRShape.hxx>

#include <Standard.hxx>
#include <Standard_Type.hxx>
#include <Standard_Transient.hxx>

#include <Prs3d_Drawer.hxx>
#include <Prs3d_Presentation.hxx>
#include <Prs3d_Projector.hxx>
#include <TopoDS_Shape.hxx>

IMPLEMENT_STANDARD_RTTIEXT(StdPrs_DeferredHLRShape, Standard_Transient)

StdPrs_DeferredHLRShape::StdPrs_DeferredHLRShape()
    : myInWork(Standard_False) {};

StdPrs_DeferredHLRShape::~StdPrs_DeferredHLRShape() {
  CancelCompute();
  Finish();
};

void StdPrs_DeferredHLRShape::Finish() {
  myInWork = Standard_False;
  myPresentation.Nullify();
  myShape.Nullify();
  myDrawer.Nullify();
  myProjector.Nullify();
}

void StdPrs_DeferredHLRShape::Add(const Handle(Prs3d_Presentation) &aPresentation,
                                  const TopoDS_Shape &aShape,
                                  const Handle(Prs3d_Drawer) &aDrawer,
                                  const Handle(Prs3d_Projector) &aProjector) {
  if (myInWork) {
    CancelCompute();
  }

  myPresentation = aPresentation;
  myShape = aShape;
  myDrawer = aDrawer;
  myProjector = aProjector;
  myInWork = true;

  DeferredCompute();
}
