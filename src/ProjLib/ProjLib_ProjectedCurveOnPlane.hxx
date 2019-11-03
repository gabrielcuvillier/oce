// Updates by: Copyright (c) 2019 Gabriel Cuvillier - Continuation Labs
//  Spin-off of ProjLib_ProjectedCurve, focused on Plane projection only. This is only used by BRep_Tool::CurveOnPlane
//  (as well as Shape_Analysis_Edge) as a replacement for ProjLib_ProjectedCurve. Doing so helps to GREATLY reduce the
//  size of binaries that do not depend on all the cases handled by the general ProjLib_ProjectedCurve.

#ifndef _ProjLib_ProjectedCurveOnPlane_HeaderFile
#define _ProjLib_ProjectedCurveOnPlane_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <Standard_Real.hxx>
#include <ProjLib_Projector.hxx>
#include <Adaptor2d_Curve2d.hxx>
#include <GeomAbs_Shape.hxx>
#include <Standard_Integer.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <Standard_Boolean.hxx>
#include <GeomAbs_CurveType.hxx>
#include <AppParCurves_Constraint.hxx>
class Adaptor3d_HSurface;
class Adaptor3d_HCurve;
class Standard_OutOfRange;
class Standard_NoSuchObject;
class Standard_DomainError;
class Standard_NotImplemented;
class Adaptor2d_HCurve2d;
class gp_Pnt2d;
class gp_Vec2d;
class gp_Lin2d;
class gp_Circ2d;
class gp_Elips2d;
class gp_Hypr2d;
class gp_Parab2d;
class Geom2d_BezierCurve;
class Geom2d_BSplineCurve;

class ProjLib_ProjectedCurveOnPlane  : public Adaptor2d_Curve2d
{
public:

  DEFINE_STANDARD_ALLOC

  //! Empty constructor, it only sets some initial values for class fields.
  Standard_EXPORT ProjLib_ProjectedCurveOnPlane();
  
  //! Constructor with initialisation field mySurface
  Standard_EXPORT ProjLib_ProjectedCurveOnPlane(const Handle(Adaptor3d_HSurface)& S);

  //! Constructor, which performs projecting.
  //! If projecting uses approximation, default parameters are used, in particular, 3d tolerance of approximation
  //! is Precision::Confusion()
  Standard_EXPORT ProjLib_ProjectedCurveOnPlane(const Handle(Adaptor3d_HSurface)& S, const Handle(Adaptor3d_HCurve)& C);
  
  //! Constructor, which performs projecting.
  //! If projecting uses approximation, 3d tolerance is Tol, default parameters are used, 
  Standard_EXPORT ProjLib_ProjectedCurveOnPlane(const Handle(Adaptor3d_HSurface)& S, const Handle(Adaptor3d_HCurve)& C, const Standard_Real Tol);
  
  //! Changes the tolerance used to project
  //! the curve on the surface
  Standard_EXPORT void Load (const Standard_Real Tolerance);
  
  //! Changes the Surface.
  Standard_EXPORT void Load (const Handle(Adaptor3d_HSurface)& S);
  
  //! Performs projecting for given curve.
  //! If projecting uses approximation, 
  //! approximation parameters can be set before by corresponding methods 
  //! SetDegree(...), SetMaxSegmets(...), SetBndPnt(...), SetMaxDist(...)
  Standard_EXPORT void Perform (const Handle(Adaptor3d_HCurve)& C);
  
  //! Set min and max possible degree of result BSpline curve2d, which is got by approximation.
  //! If theDegMin/Max < 0, algorithm uses values that are chosen depending of types curve 3d
  //! and surface.
  Standard_EXPORT void SetDegree(const Standard_Integer theDegMin, const Standard_Integer theDegMax);

  //! Set the parameter, which defines maximal value of parametric intervals the projected
  //! curve can be cut for approximation. If theMaxSegments < 0, algorithm uses default 
  //! value = 1000.
  Standard_EXPORT void SetMaxSegments(const Standard_Integer theMaxSegments);

  //! Set the parameter, which defines type of boundary condition between segments during approximation.
  //! It can be AppParCurves_PassPoint or AppParCurves_TangencyPoint.
  //! Default value is AppParCurves_TangencyPoint;
  Standard_EXPORT void SetBndPnt(const AppParCurves_Constraint theBndPnt);

  //! Set the parameter, which degines maximal possible distance between projected curve and surface.
  //! It uses only for projecting on not analytical surfaces.
  //! If theMaxDist < 0, algoritm uses default value 100.*Tolerance. 
  //! If real distance between curve and surface more then theMaxDist, algorithm stops working.
  Standard_EXPORT void SetMaxDist(const Standard_Real theMaxDist);

  Standard_EXPORT const Handle(Adaptor3d_HSurface)& GetSurface() const;
  
  Standard_EXPORT const Handle(Adaptor3d_HCurve)& GetCurve() const;
  
  //! returns the tolerance reached if an approximation
  //! is Done.
  Standard_EXPORT Standard_Real GetTolerance() const;
  
  Standard_EXPORT Standard_Real FirstParameter() const Standard_OVERRIDE;
  
  Standard_EXPORT Standard_Real LastParameter() const Standard_OVERRIDE;
  
  Standard_EXPORT GeomAbs_Shape Continuity() const Standard_OVERRIDE;
  
  //! If necessary,  breaks the  curve in  intervals  of
  //! continuity  <S>.    And  returns   the number   of
  //! intervals.
  Standard_EXPORT Standard_Integer NbIntervals (const GeomAbs_Shape S) const Standard_OVERRIDE;
  
  //! Stores in <T> the  parameters bounding the intervals
  //! of continuity <S>.
  //!
  //! The array must provide  enough room to  accomodate
  //! for the parameters. i.e. T.Length() > NbIntervals()
  Standard_EXPORT void Intervals (TColStd_Array1OfReal& T, const GeomAbs_Shape S) const Standard_OVERRIDE;
  
  //! Returns    a  curve equivalent   of  <me>  between
  //! parameters <First>  and <Last>. <Tol>  is used  to
  //! test for 3d points confusion.
  //! If <First> >= <Last>
  Standard_EXPORT Handle(Adaptor2d_HCurve2d) Trim (const Standard_Real First, const Standard_Real Last, const Standard_Real Tol) const Standard_OVERRIDE;
  
  Standard_EXPORT Standard_Boolean IsClosed() const Standard_OVERRIDE;
  
  Standard_EXPORT Standard_Boolean IsPeriodic() const Standard_OVERRIDE;
  
  Standard_EXPORT Standard_Real Period() const Standard_OVERRIDE;
  
  //! Computes the point of parameter U on the curve.
  Standard_EXPORT gp_Pnt2d Value (const Standard_Real U) const Standard_OVERRIDE;
  
  //! Computes the point of parameter U on the curve.
  Standard_EXPORT void D0 (const Standard_Real U, gp_Pnt2d& P) const Standard_OVERRIDE;
  
  //! Computes the point of parameter U on the curve with its
  //! first derivative.
  //! Raised if the continuity of the current interval
  //! is not C1.
  Standard_EXPORT void D1 (const Standard_Real U, gp_Pnt2d& P, gp_Vec2d& V) const Standard_OVERRIDE;
  

  //! Returns the point P of parameter U, the first and second
  //! derivatives V1 and V2.
  //! Raised if the continuity of the current interval
  //! is not C2.
  Standard_EXPORT void D2 (const Standard_Real U, gp_Pnt2d& P, gp_Vec2d& V1, gp_Vec2d& V2) const Standard_OVERRIDE;
  

  //! Returns the point P of parameter U, the first, the second
  //! and the third derivative.
  //! Raised if the continuity of the current interval
  //! is not C3.
  Standard_EXPORT void D3 (const Standard_Real U, gp_Pnt2d& P, gp_Vec2d& V1, gp_Vec2d& V2, gp_Vec2d& V3) const Standard_OVERRIDE;
  

  //! The returned vector gives the value of the derivative for the
  //! order of derivation N.
  //! Raised if the continuity of the current interval
  //! is not CN.
  //! Raised if N < 1.
  Standard_EXPORT gp_Vec2d DN (const Standard_Real U, const Standard_Integer N) const Standard_OVERRIDE;
  
  //! Returns the parametric  resolution corresponding
  //! to the real space resolution <R3d>.
  Standard_EXPORT Standard_Real Resolution (const Standard_Real R3d) const Standard_OVERRIDE;
  
  //! Returns  the  type of the   curve  in the  current
  //! interval :   Line,   Circle,   Ellipse, Hyperbola,
  //! Parabola, BezierCurve, BSplineCurve, OtherCurve.
  Standard_EXPORT GeomAbs_CurveType GetType() const Standard_OVERRIDE;
  
  Standard_EXPORT gp_Lin2d Line() const Standard_OVERRIDE;
  
  Standard_EXPORT gp_Circ2d Circle() const Standard_OVERRIDE;
  
  Standard_EXPORT gp_Elips2d Ellipse() const Standard_OVERRIDE;
  
  Standard_EXPORT gp_Hypr2d Hyperbola() const Standard_OVERRIDE;
  
  Standard_EXPORT gp_Parab2d Parabola() const Standard_OVERRIDE;
  
  Standard_EXPORT Standard_Integer Degree() const Standard_OVERRIDE;
  
  Standard_EXPORT Standard_Boolean IsRational() const Standard_OVERRIDE;
  
  Standard_EXPORT Standard_Integer NbPoles() const Standard_OVERRIDE;
  
  Standard_EXPORT Standard_Integer NbKnots() const Standard_OVERRIDE;
  
  //! Warning ! This will  NOT make a copy  of the -- Bezier Curve -
  //! If you want to modify -- the Curve  please make a copy
  //! yourself --  Also it will  NOT trim the surface  to --
  //! myFirst/Last.
  Standard_EXPORT Handle(Geom2d_BezierCurve) Bezier() const Standard_OVERRIDE;
  
  //! Warning ! This will NOT make a copy of the BSpline Curve - If
  //! you want to  modify the   Curve  please make a   copy
  //! yourself Also it   will  NOT  trim   the surface   to
  //! myFirst/Last.
  Standard_EXPORT Handle(Geom2d_BSplineCurve) BSpline() const Standard_OVERRIDE;




protected:





private:



  Standard_Real myTolerance;
  Handle(Adaptor3d_HSurface) mySurface;
  Handle(Adaptor3d_HCurve) myCurve;
  ProjLib_Projector myResult;
  Standard_Integer myDegMin;
  Standard_Integer myDegMax;
  Standard_Integer myMaxSegments;
  Standard_Real myMaxDist;
  AppParCurves_Constraint myBndPnt;
};

#endif // _ProjLib_ProjectedCurveOnPlane_HeaderFile
