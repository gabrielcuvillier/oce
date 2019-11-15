// Updates by: Copyright (c) 2019 Gabriel Cuvillier - Continuation Labs

#include <GeomAbs_SurfaceType.hxx>
#include <Standard_NoSuchObject.hxx>
#include <Standard_NotImplemented.hxx>
#include <ProjLib_ProjectedCurveOnPlane.hxx>
#include <ProjLib_Projector.hxx>
#include <Adaptor3d_HCurve.hxx>
#include <Adaptor3d_HSurface.hxx>
#include <ProjLib_Plane.hxx>
#include <Precision.hxx>
#include <Geom2d_BSplineCurve.hxx>
#include <Geom2d_BezierCurve.hxx>
#include <gp_Vec2d.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <NCollection_DataMap.hxx>
#include <ProjLib_ComputeApprox.hxx>
#include <Geom2dConvert.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2dConvert_CompCurveToBSplineCurve.hxx>
#include <ElCLib.hxx>
#include <GeomLib.hxx>

//=======================================================================
//function : ExtendC2d
//purpose  :
//=======================================================================

static void ExtendC2d(Handle(Geom2d_BSplineCurve) & aRes,
const Standard_Real /*t*/,
const Standard_Real /*dt*/,
const Standard_Real u1,
const Standard_Real u2,
const Standard_Real v1,
const Standard_Real v2,
const Standard_Integer FirstOrLast,
const Standard_Integer NumberOfSingularCase)
{
  Standard_Real theParam = (FirstOrLast == 0) ? aRes->FirstParameter()
                                              : aRes->LastParameter();

  gp_Pnt2d aPBnd;
  gp_Vec2d aVBnd;
  gp_Dir2d aDBnd;
  Handle(Geom2d_TrimmedCurve)aSegment;
  Geom2dConvert_CompCurveToBSplineCurve aCompCurve(aRes, Convert_RationalC1);
  Standard_Real aTol = Precision::Confusion();

  aRes->D1(theParam, aPBnd, aVBnd);
  aDBnd.SetXY(aVBnd.XY());
  gp_Lin2d aLin(aPBnd, aDBnd); //line in direction of derivative

  gp_Pnt2d thePole;
  gp_Dir2d theBoundDir;
  switch (NumberOfSingularCase)
  {
  case 1:
  {
      thePole.SetCoord(u1, v1);
      theBoundDir.SetCoord(0., 1.);
    break;
  }
  case 2:
  {
    thePole.SetCoord(u2, v1);
    theBoundDir.SetCoord(0., 1.);
    break;
  }
  case 3:
  {
    thePole.SetCoord(u1, v1);
    theBoundDir.SetCoord(1., 0.);
    break;
  }
  case 4:
  {
    thePole.SetCoord(u1, v2);
    theBoundDir.SetCoord(1., 0.);
    break;
  }
  }

  gp_Lin2d BoundLin(thePole, theBoundDir); //one of the bounds of rectangle
  Standard_Real ParOnLin = 0.;
  if (theBoundDir.IsParallel(aDBnd,100.*Precision::Angular()))
  {
    ParOnLin = ElCLib::Parameter(aLin, thePole);
  }
  else
  {
    Standard_Real U1x = BoundLin.Direction().X();
    Standard_Real U1y = BoundLin.Direction().Y();
    Standard_Real U2x = aLin.Direction().X();
    Standard_Real U2y = aLin.Direction().Y();
    Standard_Real Uo21x = aLin.Location().X() - BoundLin.Location().X();
    Standard_Real Uo21y = aLin.Location().Y() - BoundLin.Location().Y();

    Standard_Real D = U1y * U2x - U1x * U2y;

    ParOnLin = (Uo21y * U1x - Uo21x * U1y) / D; //parameter of intersection point
  }

  Handle(Geom2d_Line)
  aSegLine = new Geom2d_Line(aLin);
  aSegment = (FirstOrLast == 0) ?
             new Geom2d_TrimmedCurve(aSegLine, ParOnLin, 0.) :
             new Geom2d_TrimmedCurve(aSegLine, 0., ParOnLin);

  aCompCurve.Add(aSegment, aTol);
  aRes = aCompCurve.BSplineCurve();
}

//=======================================================================
//function : Project
//purpose  : 
//=======================================================================

static void Project(ProjLib_Projector& P, Handle(Adaptor3d_HCurve)& C)
{
  GeomAbs_CurveType CType = C->GetType();
  switch (CType) {
    case GeomAbs_Line:
      P.Project(C->Line());
      break;
    case GeomAbs_Circle:
      P.Project(C->Circle());
      break;
    case GeomAbs_Ellipse:
      P.Project(C->Ellipse());
      break;
    case GeomAbs_Hyperbola:
      P.Project(C->Hyperbola());
      break;
    case GeomAbs_Parabola:
      P.Project(C->Parabola());
      break;
    case GeomAbs_BSplineCurve:
    case GeomAbs_BezierCurve:
    case GeomAbs_OffsetCurve:
    case GeomAbs_OtherCurve:    // try the approximation
      break;
    default:
      throw Standard_NoSuchObject(" ");
  }
}

//=======================================================================
//function : ProjLib_ProjectedCurveOnPlane
//purpose  : 
//=======================================================================

ProjLib_ProjectedCurveOnPlane::ProjLib_ProjectedCurveOnPlane() :
  myTolerance(Precision::Confusion()),
  myDegMin(-1), myDegMax(-1),
  myMaxSegments(-1),
  myMaxDist(-1.),
  myBndPnt(AppParCurves_TangencyPoint)
{
}


//=======================================================================
//function : ProjLib_ProjectedCurveOnPlane
//purpose  : 
//=======================================================================

ProjLib_ProjectedCurveOnPlane::ProjLib_ProjectedCurveOnPlane
(const Handle(Adaptor3d_HSurface)& S) :
  myTolerance(Precision::Confusion()),
  myDegMin(-1), myDegMax(-1),
  myMaxSegments(-1),
  myMaxDist(-1.),
  myBndPnt(AppParCurves_TangencyPoint)
{
  Load(S);
}


//=======================================================================
//function : ProjLib_ProjectedCurveOnPlane
//purpose  : 
//=======================================================================

ProjLib_ProjectedCurveOnPlane::ProjLib_ProjectedCurveOnPlane
(const Handle(Adaptor3d_HSurface)& S,
 const Handle(Adaptor3d_HCurve)& C) :
  myTolerance(Precision::Confusion()),
  myDegMin(-1), myDegMax(-1),
  myMaxSegments(-1),
  myMaxDist(-1.),
  myBndPnt(AppParCurves_TangencyPoint)
{
  Load(S);
  Perform(C);
}


//=======================================================================
//function : ProjLib_ProjectedCurveOnPlane
//purpose  : 
//=======================================================================

ProjLib_ProjectedCurveOnPlane::ProjLib_ProjectedCurveOnPlane
(const Handle(Adaptor3d_HSurface)& S,
 const Handle(Adaptor3d_HCurve)&   C,
 const Standard_Real             Tol) :
  myTolerance(Max(Tol, Precision::Confusion())),
  myDegMin(-1), myDegMax(-1),
  myMaxSegments(-1),
  myMaxDist(-1.),
  myBndPnt(AppParCurves_TangencyPoint)
{
  Load(S);
  Perform(C);
}


//=======================================================================
//function : Load
//purpose  : 
//=======================================================================

void ProjLib_ProjectedCurveOnPlane::Load(const Handle(Adaptor3d_HSurface)& S)
{
  mySurface = S ;
}

//=======================================================================
//function : Load
//purpose  : 
//=======================================================================

void ProjLib_ProjectedCurveOnPlane::Load(const Standard_Real theTol)
{
  myTolerance = theTol;
}

//=======================================================================
//function : Perform
//purpose  : 
//=======================================================================

void ProjLib_ProjectedCurveOnPlane::Perform(const Handle(Adaptor3d_HCurve)& C)
{
  myTolerance = Max(myTolerance, Precision::Confusion());
  myCurve = C;
  Standard_Real FirstPar = C->FirstParameter();
  Standard_Real LastPar  = C->LastParameter();
  GeomAbs_SurfaceType SType = mySurface->GetType();
  GeomAbs_CurveType   CType = myCurve->GetType();
  Standard_Boolean IsTrimmed[2] = { Standard_False, Standard_False };
  Standard_Integer SingularCase[2];
  const Standard_Real eps = 0.01;
  Standard_Real dt = (LastPar - FirstPar) * eps;
  Standard_Real U1 = 0.0, U2 = 0.0, V1 = 0.0, V2 = 0.0;
  U1 = mySurface->FirstUParameter();
  U2 = mySurface->LastUParameter();
  V1 = mySurface->FirstVParameter();
  V2 = mySurface->LastVParameter();

  switch (SType)
  {
    case GeomAbs_Plane:
      {
        ProjLib_Plane P(mySurface->Plane());
        Project(P,myCurve);
        myResult = P;
      }
      break;
    default:
      return;
  }

  if ( !myResult.IsDone() )
  {
    // Use advanced analytical projector if base analytical projection failed.
    ProjLib_ComputeApprox Comp;
    Comp.SetTolerance(myTolerance);
    Comp.SetDegree(myDegMin, myDegMax);
    Comp.SetMaxSegments(myMaxSegments);
    Comp.SetBndPnt(myBndPnt);
    Comp.Perform(myCurve, mySurface);
    if (Comp.Bezier().IsNull() && Comp.BSpline().IsNull())
      return; // advanced projector has been failed too
    myResult.Done();
    Handle(Geom2d_BSplineCurve) aRes;
    if (Comp.BSpline().IsNull())
    {
      aRes = Geom2dConvert::CurveToBSplineCurve(Comp.Bezier());
    }
    else
    {
      aRes = Comp.BSpline();
    }
    if ((IsTrimmed[0] || IsTrimmed[1]))
    {
      if (IsTrimmed[0])
      {
        //Add segment before start of curve
        Standard_Real f = myCurve->FirstParameter();
        ExtendC2d(aRes, f, -dt, U1, U2, V1, V2, 0, SingularCase[0]);
      }
      if (IsTrimmed[1])
      {
        //Add segment after end of curve
        Standard_Real l = myCurve->LastParameter();
        ExtendC2d(aRes, l, dt, U1, U2, V1, V2, 1, SingularCase[1]);
      }
      Handle(Geom2d_Curve) NewCurve2d;
      GeomLib::SameRange(Precision::PConfusion(), aRes,
      aRes->FirstParameter(), aRes->LastParameter(),
      FirstPar, LastPar, NewCurve2d);
      aRes = Handle(Geom2d_BSplineCurve)::DownCast(NewCurve2d);
      myResult.SetBSpline(aRes);
      myResult.SetType(GeomAbs_BSplineCurve);
    }
    else
    {
      // set the type
      if (SType == GeomAbs_Plane && CType == GeomAbs_BezierCurve)
      {
        myResult.SetType(GeomAbs_BezierCurve);
        myResult.SetBezier(Comp.Bezier());
      }
      else
      {
        myResult.SetType(GeomAbs_BSplineCurve);
        myResult.SetBSpline(Comp.BSpline());
      }
    }
    // set the periodicity flag
    if (SType == GeomAbs_Plane        &&
      CType == GeomAbs_BSplineCurve &&
      myCurve->IsPeriodic())
    {
      myResult.SetPeriodic();
    }
    myTolerance = Comp.Tolerance();
  }

  // Planes are never periodic, so don't check result curve to be in params space
}

//=======================================================================
//function : SetDegree
//purpose  : 
//=======================================================================
void ProjLib_ProjectedCurveOnPlane::SetDegree(const Standard_Integer theDegMin,
                                       const Standard_Integer theDegMax)
{
  myDegMin = theDegMin;
  myDegMax = theDegMax;
}
//=======================================================================
//function : SetMaxSegments
//purpose  : 
//=======================================================================
void ProjLib_ProjectedCurveOnPlane::SetMaxSegments(const Standard_Integer theMaxSegments)
{
  myMaxSegments = theMaxSegments;
}

//=======================================================================
//function : SetBndPnt
//purpose  : 
//=======================================================================
void ProjLib_ProjectedCurveOnPlane::SetBndPnt(const AppParCurves_Constraint theBndPnt)
{
  myBndPnt = theBndPnt;
}

//=======================================================================
//function : SetMaxDist
//purpose  : 
//=======================================================================
void ProjLib_ProjectedCurveOnPlane::SetMaxDist(const Standard_Real theMaxDist)
{
  myMaxDist = theMaxDist;
}

//=======================================================================
//function : GetSurface
//purpose  : 
//=======================================================================

const Handle(Adaptor3d_HSurface)& ProjLib_ProjectedCurveOnPlane::GetSurface() const
{
  return mySurface;
}


//=======================================================================
//function : GetCurve
//purpose  : 
//=======================================================================

const Handle(Adaptor3d_HCurve)& ProjLib_ProjectedCurveOnPlane::GetCurve() const
{
  return myCurve;
}


//=======================================================================
//function : GetTolerance
//purpose  : 
//=======================================================================

Standard_Real ProjLib_ProjectedCurveOnPlane::GetTolerance() const
{
  return myTolerance;
}


//=======================================================================
//function : FirstParameter
//purpose  : 
//=======================================================================

Standard_Real ProjLib_ProjectedCurveOnPlane::FirstParameter() const
{
  return myCurve->FirstParameter();
}


//=======================================================================
//function : LastParameter
//purpose  : 
//=======================================================================

Standard_Real ProjLib_ProjectedCurveOnPlane::LastParameter() const
{
  return myCurve->LastParameter();
}


//=======================================================================
//function : Continuity
//purpose  : 
//=======================================================================

GeomAbs_Shape ProjLib_ProjectedCurveOnPlane::Continuity() const
{
  throw Standard_NotImplemented ("ProjLib_ProjectedCurveOnPlane::Continuity() - method is not implemented");
}


//=======================================================================
//function : NbIntervals
//purpose  : 
//=======================================================================

Standard_Integer ProjLib_ProjectedCurveOnPlane::NbIntervals(const GeomAbs_Shape ) const
{
  throw Standard_NotImplemented ("ProjLib_ProjectedCurveOnPlane::NbIntervals() - method is not implemented");
}


//=======================================================================
//function : Intervals
//purpose  : 
//=======================================================================

//void ProjLib_ProjectedCurveOnPlane::Intervals(TColStd_Array1OfReal&  T,
void ProjLib_ProjectedCurveOnPlane::Intervals(TColStd_Array1OfReal&  ,
				       const GeomAbs_Shape ) const
{
  throw Standard_NotImplemented ("ProjLib_ProjectedCurveOnPlane::Intervals() - method is not implemented");
}


//=======================================================================
//function : IsClosed
//purpose  : 
//=======================================================================

Standard_Boolean ProjLib_ProjectedCurveOnPlane::IsClosed() const
{
  throw Standard_NotImplemented ("ProjLib_ProjectedCurveOnPlane::IsClosed() - method is not implemented");
}


//=======================================================================
//function : IsPeriodic
//purpose  : 
//=======================================================================

Standard_Boolean ProjLib_ProjectedCurveOnPlane::IsPeriodic() const
{
  return myResult.IsPeriodic();
}


//=======================================================================
//function : Period
//purpose  : 
//=======================================================================

Standard_Real ProjLib_ProjectedCurveOnPlane::Period() const
{
  throw Standard_NotImplemented ("ProjLib_ProjectedCurveOnPlane::Period() - method is not implemented");
}


//=======================================================================
//function : Value
//purpose  : 
//=======================================================================

gp_Pnt2d ProjLib_ProjectedCurveOnPlane::Value(const Standard_Real ) const
{
  throw Standard_NotImplemented ("ProjLib_ProjectedCurveOnPlane::Value() - method is not implemented");
}


//=======================================================================
//function : D0
//purpose  : 
//=======================================================================

void ProjLib_ProjectedCurveOnPlane::D0(const Standard_Real , gp_Pnt2d& ) const
{
  throw Standard_NotImplemented ("ProjLib_ProjectedCurveOnPlane::D0() - method is not implemented");
}


//=======================================================================
//function : D1
//purpose  : 
//=======================================================================

void ProjLib_ProjectedCurveOnPlane::D1(const Standard_Real ,
			              gp_Pnt2d&     ,
                                      gp_Vec2d&     ) const
{
  throw Standard_NotImplemented ("ProjLib_ProjectedCurveOnPlane::D1() - method is not implemented");
}


//=======================================================================
//function : D2
//purpose  : 
//=======================================================================

void ProjLib_ProjectedCurveOnPlane::D2(const Standard_Real ,
			              gp_Pnt2d&     ,
                                      gp_Vec2d&     ,
                                      gp_Vec2d&     ) const
{
  throw Standard_NotImplemented ("ProjLib_ProjectedCurveOnPlane::D2() - method is not implemented");
}


//=======================================================================
//function : D3
//purpose  : 
//=======================================================================

void ProjLib_ProjectedCurveOnPlane::D3(const Standard_Real,
				      gp_Pnt2d&,
			              gp_Vec2d&,
				      gp_Vec2d&,
			              gp_Vec2d&) const
{
  throw Standard_NotImplemented ("ProjLib_ProjectedCurveOnPlane::D3() - method is not implemented");
}


//=======================================================================
//function : DN
//purpose  : 
//=======================================================================

gp_Vec2d ProjLib_ProjectedCurveOnPlane::DN(const Standard_Real,
				    const Standard_Integer) const
{
  throw Standard_NotImplemented ("ProjLib_ProjectedCurveOnPlane::DN() - method is not implemented");
}


//=======================================================================
//function : Resolution
//purpose  : 
//=======================================================================

Standard_Real ProjLib_ProjectedCurveOnPlane::Resolution(const Standard_Real) const
{
  throw Standard_NotImplemented ("ProjLib_ProjectedCurveOnPlane::Resolution() - method is not implemented");
}


//=======================================================================
//function : GetType
//purpose  : 
//=======================================================================

GeomAbs_CurveType ProjLib_ProjectedCurveOnPlane::GetType() const
{
  return myResult.GetType();
}


//=======================================================================
//function : Line
//purpose  : 
//=======================================================================

gp_Lin2d ProjLib_ProjectedCurveOnPlane::Line() const
{
  return myResult.Line();
}


//=======================================================================
//function : Circle
//purpose  : 
//=======================================================================

gp_Circ2d ProjLib_ProjectedCurveOnPlane::Circle() const
{
  return myResult.Circle();
}


//=======================================================================
//function : Ellipse
//purpose  : 
//=======================================================================

gp_Elips2d ProjLib_ProjectedCurveOnPlane::Ellipse() const
{
  return myResult.Ellipse();
}


//=======================================================================
//function : Hyperbola
//purpose  : 
//=======================================================================

gp_Hypr2d ProjLib_ProjectedCurveOnPlane::Hyperbola() const
{
  return myResult.Hyperbola();
}


//=======================================================================
//function : Parabola
//purpose  : 
//=======================================================================

gp_Parab2d ProjLib_ProjectedCurveOnPlane::Parabola() const
{
  return myResult.Parabola();
}



//=======================================================================
//function : Degree
//purpose  : 
//=======================================================================

Standard_Integer ProjLib_ProjectedCurveOnPlane::Degree() const
{
  Standard_NoSuchObject_Raise_if
    ( (GetType() != GeomAbs_BSplineCurve) &&
      (GetType() != GeomAbs_BezierCurve),
     "ProjLib_ProjectedCurveOnPlane:Degree");
  if (GetType() == GeomAbs_BSplineCurve) {
    return myResult.BSpline()->Degree();
  }
  else if (GetType() == GeomAbs_BezierCurve) {
    return myResult.Bezier()->Degree();
  }

  // portage WNT
  return 0;
}

//=======================================================================
//function : IsRational
//purpose  : 
//=======================================================================

Standard_Boolean ProjLib_ProjectedCurveOnPlane::IsRational() const
{
  Standard_NoSuchObject_Raise_if
    ( (GetType() != GeomAbs_BSplineCurve) &&
      (GetType() != GeomAbs_BezierCurve),
     "ProjLib_ProjectedCurveOnPlane:IsRational");
  if (GetType() == GeomAbs_BSplineCurve) {
    return myResult.BSpline()->IsRational();
  }
  else if (GetType() == GeomAbs_BezierCurve) {
    return myResult.Bezier()->IsRational();
  }
  // portage WNT
  return Standard_False;
}

//=======================================================================
//function : NbPoles
//purpose  : 
//=======================================================================

Standard_Integer ProjLib_ProjectedCurveOnPlane::NbPoles() const
{
  Standard_NoSuchObject_Raise_if
    ( (GetType() != GeomAbs_BSplineCurve) &&
      (GetType() != GeomAbs_BezierCurve)
     ,"ProjLib_ProjectedCurveOnPlane:NbPoles"  );
  if (GetType() == GeomAbs_BSplineCurve) {
    return myResult.BSpline()->NbPoles();
  }
  else if (GetType() == GeomAbs_BezierCurve) {
    return myResult.Bezier()->NbPoles();
  }

  // portage WNT
  return 0;
}

//=======================================================================
//function : NbKnots
//purpose  : 
//=======================================================================

Standard_Integer ProjLib_ProjectedCurveOnPlane::NbKnots() const
{
  Standard_NoSuchObject_Raise_if ( GetType() != GeomAbs_BSplineCurve,
				  "ProjLib_ProjectedCurveOnPlane:NbKnots");
  return myResult.BSpline()->NbKnots();
}

//=======================================================================
//function : Bezier
//purpose  : 
//=======================================================================

Handle(Geom2d_BezierCurve) ProjLib_ProjectedCurveOnPlane::Bezier() const
{
 return myResult.Bezier() ;
}

//=======================================================================
//function : BSpline
//purpose  : 
//=======================================================================

Handle(Geom2d_BSplineCurve) ProjLib_ProjectedCurveOnPlane::BSpline() const
{
 return myResult.BSpline() ;
}
//=======================================================================
//function : Trim
//purpose  : 
//=======================================================================

Handle(Adaptor2d_HCurve2d) ProjLib_ProjectedCurveOnPlane::Trim
//(const Standard_Real First,
// const Standard_Real Last,
// const Standard_Real Tolerance) const 
(const Standard_Real ,
 const Standard_Real ,
 const Standard_Real ) const
{
  throw Standard_NotImplemented ("ProjLib_ProjectedCurveOnPlane::Trim() - method is not implemented");
}
