// Created on: 1994-12-09
// Created by: Jacques GOUSSARD
// Copyright (c) 1994-1999 Matra Datavision
// Copyright (c) 1999-2014 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

// NOTE:
// GAB 2019: Copied from BRepBuilderAPI_Transform.cxx, so that some AIS classes does not depends on TKTopAlgo, but rather on TKTopBase

#include <BRepLib_Transform.hxx>
#include <BRepTools_TrsfModification.hxx>
#include <gp.hxx>
#include <gp_Trsf.hxx>
#include <Standard_NoSuchObject.hxx>
#include <TopoDS_Shape.hxx>

//=======================================================================
//function : BRepLib_Transform
//purpose  : 
//=======================================================================
BRepLib_Transform::BRepLib_Transform (const gp_Trsf& T) :
    myTrsf(T)
{
  myModification = new BRepTools_TrsfModification(T);
}


//=======================================================================
//function : BRepLib_Transform
//purpose  : 
//=======================================================================

BRepLib_Transform::BRepLib_Transform (const TopoDS_Shape& S,
                                                    const gp_Trsf& T,
                                                    const Standard_Boolean Copy) :
    myTrsf(T)
{
  myModification = new BRepTools_TrsfModification(T);
  Perform(S,Copy);
}



//=======================================================================
//function : Perform
//purpose  : 
//=======================================================================

void BRepLib_Transform::Perform(const TopoDS_Shape& S,
                                       const Standard_Boolean Copy)
{
//  myUseModif = Copy || myTrsf.IsNegative(); bug gp_Trsf.
  myUseModif = Copy ||
      myTrsf.ScaleFactor()*myTrsf.HVectorialPart().Determinant() < 0. ||
      Abs(Abs(myTrsf.ScaleFactor()) - 1) > gp::Resolution();
  if (myUseModif) {
    Handle(BRepTools_TrsfModification) theModif =
                                           Handle(BRepTools_TrsfModification)::DownCast(myModification);
    theModif->Trsf() = myTrsf;
    DoModif(S,myModification);
  }
  else {
    myLocation = myTrsf;
    myShape = S.Moved(myLocation);
    Done();
  }

}

//=======================================================================
//function : ModifiedShape
//purpose  : 
//=======================================================================

TopoDS_Shape BRepLib_Transform::ModifiedShape
    (const TopoDS_Shape& S) const
{
  if (myUseModif) {
    return myModifier.ModifiedShape(S);
  }
  return S.Moved (myLocation);
}


//=======================================================================
//function : Modified
//purpose  : 
//=======================================================================

const TopTools_ListOfShape& BRepLib_Transform::Modified
    (const TopoDS_Shape& F)
{
  if (!myUseModif) {
    myGenFaces.Clear();
    myGenFaces.Append(F.Moved(myLocation));
    return myGenFaces;
  }
  return BRepLib_ModifyShape::Modified(F);
}

