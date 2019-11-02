// Copyright (c) 2019 Gabriel Cuvillier - Continuation Labs
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.

#ifndef _StdPrs_DeferredHLRShape_HeaderFile
#define _StdPrs_DeferredHLRShape_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>
#include <Standard_Transient.hxx>

#include <Prs3d_Drawer.hxx>
#include <Prs3d_Presentation.hxx>
#include <Prs3d_Projector.hxx>
#include <TopoDS_Shape.hxx>

class StdPrs_DeferredHLRShape;
DEFINE_STANDARD_HANDLE(StdPrs_DeferredHLRShape, Standard_Transient)

class StdPrs_DeferredHLRShape : public Standard_Transient {
 public:
  StdPrs_DeferredHLRShape();
  ~StdPrs_DeferredHLRShape();

  virtual void DeferredCompute() = 0;
  virtual void CancelCompute() = 0;

  virtual void Add( const Handle(Prs3d_Presentation) & aPresentation,
                    const TopoDS_Shape &aShape,
                    const Handle(Prs3d_Drawer)& aDrawer,
                    const Handle(Prs3d_Projector) & aProjector);

  virtual void Finish();

 protected:
  Standard_Boolean myInWork;
  Handle(Prs3d_Presentation) myPresentation;
  TopoDS_Shape myShape;
  Handle(Prs3d_Drawer) myDrawer;
  Handle(Prs3d_Projector) myProjector;

 private:
  DEFINE_STANDARD_RTTIEXT(StdPrs_DeferredHLRShape, Standard_Transient)
};

#endif // _StdPrs_DeferredHLRShape_HeaderFile
