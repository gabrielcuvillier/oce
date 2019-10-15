// Copyright (c) 1995-1999 Matra Datavision
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


#include <Bnd_Box.hxx>
#include <BRep_Polygon3D.hxx>
#include <BRep_Tool.hxx>
#include <BRepBndLibApprox.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Surface.hxx>
#include <Poly_Polygon3D.hxx>
#include <Poly_PolygonOnTriangulation.hxx>
#include <Poly_Triangulation.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <TColStd_HArray1OfInteger.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <Adaptor3d_HCurve.hxx>
#include <Adaptor3d_HSurface.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_BezierSurface.hxx>
#include <Bnd_Box2d.hxx>
#include <ElSLib.hxx>
#include <ElCLib.hxx>
#include <Geom_Plane.hxx>
#include <Extrema_ExtSS.hxx>

//
//=======================================================================
//function : Add
//purpose  : Add a shape bounding to a box
//=======================================================================
void BRepBndLibApprox::Add(const TopoDS_Shape& S, Bnd_Box& B)
{
  TopExp_Explorer ex;

  // Add the faces
  TopLoc_Location l, aDummyLoc;
  Standard_Integer i, nbNodes;

  for (ex.Init(S,TopAbs_FACE); ex.More(); ex.Next()) {
    const TopoDS_Face& F = TopoDS::Face(ex.Current());
    const Handle(Poly_Triangulation)& T = BRep_Tool::Triangulation(F, l);
    if (!T.IsNull())
    {
      nbNodes = T->NbNodes();
      const TColgp_Array1OfPnt& Nodes = T->Nodes();
      for (i = 1; i <= nbNodes; i++) {
        if (l.IsIdentity()) B.Add(Nodes(i));
        else B.Add(Nodes(i).Transformed(l));
      }
      //       B.Enlarge(T->Deflection());
      B.Enlarge(T->Deflection() + BRep_Tool::Tolerance(F));
    }
  }

  // Add the edges not in faces
  Handle(TColStd_HArray1OfInteger) HIndices;
  Handle(Poly_PolygonOnTriangulation) Poly;
  Handle(Poly_Triangulation) T;
  for (ex.Init(S,TopAbs_EDGE,TopAbs_FACE); ex.More(); ex.Next())
  {
    const TopoDS_Edge& E = TopoDS::Edge(ex.Current());
    Handle(Poly_Polygon3D) P3d = BRep_Tool::Polygon3D(E, l);
    if (!P3d.IsNull())
    {
      const TColgp_Array1OfPnt& Nodes = P3d->Nodes();
      nbNodes = P3d->NbNodes();
      for (i = 1; i <= nbNodes; i++)
      {
        if (l.IsIdentity()) B.Add(Nodes(i));
        else B.Add(Nodes(i).Transformed(l));
      }
      //       B.Enlarge(P3d->Deflection());
      B.Enlarge(P3d->Deflection() + BRep_Tool::Tolerance(E));
    }
    else
    {
      BRep_Tool::PolygonOnTriangulation(E, Poly, T, l);
      if (!Poly.IsNull())
      {
        const TColStd_Array1OfInteger& Indices = Poly->Nodes();
        const TColgp_Array1OfPnt& Nodes = T->Nodes();
        nbNodes = Indices.Length();
        for (i = 1; i <= nbNodes; i++)
        {
          if (l.IsIdentity()) B.Add(Nodes(Indices(i)));
          else B.Add(Nodes(Indices(i)).Transformed(l));
        }
        // 	B.Enlarge(T->Deflection());
        B.Enlarge(Poly->Deflection() + BRep_Tool::Tolerance(E));
      }
    }
  }

  // Add the vertices not in edges

  for (ex.Init(S,TopAbs_VERTEX,TopAbs_EDGE); ex.More(); ex.Next()) {
    B.Add(BRep_Tool::Pnt(TopoDS::Vertex(ex.Current())));
    B.Enlarge(BRep_Tool::Tolerance(TopoDS::Vertex(ex.Current())));
  }
}

