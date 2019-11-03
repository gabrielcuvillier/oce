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
  GeomAbs_SurfaceType SType = mySurface->GetType();
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
      break;
  }

  Standard_Boolean isPeriodic[] = {mySurface->IsUPeriodic(),
                                   mySurface->IsVPeriodic()};
  if (myResult.IsDone() &&
     (isPeriodic[0] || isPeriodic[1]))
  {
    // Check result curve to be in params space.

    // U and V parameters space correspondingly.
    const Standard_Real aSurfFirstPar[2] = {mySurface->FirstUParameter(),
                                            mySurface->FirstVParameter()};
    Standard_Real aSurfPeriod[2] = {0.0, 0.0};
    if (isPeriodic[0])
      aSurfPeriod[0] = mySurface->UPeriod();
    if (isPeriodic[1])
      aSurfPeriod[1] = mySurface->VPeriod();

    for(Standard_Integer anIdx = 1; anIdx <= 2; anIdx++)
    {
      if (!isPeriodic[anIdx - 1])
        continue;

      if (myResult.GetType() == GeomAbs_BSplineCurve)
      {
        NCollection_DataMap<Standard_Integer, Standard_Integer> aMap; 
        Handle(Geom2d_BSplineCurve) aRes = myResult.BSpline();
        const Standard_Integer aDeg = aRes->Degree();

        for(Standard_Integer aKnotIdx = aRes->FirstUKnotIndex();
                             aKnotIdx < aRes->LastUKnotIndex();
                             aKnotIdx++)
        {
          const Standard_Real aFirstParam = aRes->Knot(aKnotIdx);
          const Standard_Real aLastParam  = aRes->Knot(aKnotIdx + 1);

          for(Standard_Integer anIntIdx = 0; anIntIdx <= aDeg; anIntIdx++)
          {
            const Standard_Real aCurrParam = aFirstParam + (aLastParam - aFirstParam) * anIntIdx / (aDeg + 1.0);
            gp_Pnt2d aPnt2d;
            aRes->D0(aCurrParam, aPnt2d);

            Standard_Integer aMapKey = Standard_Integer ((aPnt2d.Coord(anIdx) - aSurfFirstPar[anIdx - 1]) / aSurfPeriod[anIdx - 1]);

            if (aPnt2d.Coord(anIdx) - aSurfFirstPar[anIdx - 1] < 0.0)
              aMapKey--;

            if (aMap.IsBound(aMapKey))
              aMap.ChangeFind(aMapKey)++;
            else
              aMap.Bind(aMapKey, 1);
          }
        }

        Standard_Integer aMaxPoints = 0, aMaxIdx = 0;
        NCollection_DataMap<Standard_Integer, Standard_Integer>::Iterator aMapIter(aMap);
        for( ; aMapIter.More(); aMapIter.Next())
        {
          if (aMapIter.Value() > aMaxPoints)
          {
            aMaxPoints = aMapIter.Value();
            aMaxIdx = aMapIter.Key();
          }
        }
        if (aMaxIdx != 0)
        {
          gp_Pnt2d aFirstPnt = aRes->Value(aRes->FirstParameter());
          gp_Pnt2d aSecondPnt = aFirstPnt;
          aSecondPnt.SetCoord(anIdx, aFirstPnt.Coord(anIdx) - aSurfPeriod[anIdx - 1] * aMaxIdx);
          aRes->Translate(gp_Vec2d(aFirstPnt, aSecondPnt));
        }
      }

      if (myResult.GetType() == GeomAbs_Line)
      {
        Standard_Real aT1 = myCurve->FirstParameter();
        Standard_Real aT2 = myCurve->LastParameter();

        if (anIdx == 1)
        {
          // U param space.
          myResult.UFrame(aT1, aT2, aSurfFirstPar[anIdx - 1], aSurfPeriod[anIdx - 1]);
        }
        else
        {
          // V param space.
          myResult.VFrame(aT1, aT2, aSurfFirstPar[anIdx - 1], aSurfPeriod[anIdx - 1]);
        }
      }
    }
  }
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
