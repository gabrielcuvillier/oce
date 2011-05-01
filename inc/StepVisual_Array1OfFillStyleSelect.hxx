// This file is generated by WOK (CPPExt).
// Please do not edit this file; modify original file instead.
// The copyright and license terms as defined for the original file apply to 
// this header file considered to be the "object code" form of the original source.

#ifndef _StepVisual_Array1OfFillStyleSelect_HeaderFile
#define _StepVisual_Array1OfFillStyleSelect_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif

#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
#ifndef _Standard_Address_HeaderFile
#include <Standard_Address.hxx>
#endif
#ifndef _Standard_Boolean_HeaderFile
#include <Standard_Boolean.hxx>
#endif
class Standard_RangeError;
class Standard_DimensionMismatch;
class Standard_OutOfRange;
class Standard_OutOfMemory;
class StepVisual_FillStyleSelect;



class StepVisual_Array1OfFillStyleSelect  {
public:

  void* operator new(size_t,void* anAddress) 
  {
    return anAddress;
  }
  void* operator new(size_t size) 
  {
    return Standard::Allocate(size); 
  }
  void  operator delete(void *anAddress) 
  {
    if (anAddress) Standard::Free((Standard_Address&)anAddress); 
  }

  
  Standard_EXPORT   StepVisual_Array1OfFillStyleSelect(const Standard_Integer Low,const Standard_Integer Up);
  
  Standard_EXPORT   StepVisual_Array1OfFillStyleSelect(const StepVisual_FillStyleSelect& Item,const Standard_Integer Low,const Standard_Integer Up);
  
  Standard_EXPORT     void Init(const StepVisual_FillStyleSelect& V) ;
  
  Standard_EXPORT     void Destroy() ;
~StepVisual_Array1OfFillStyleSelect()
{
  Destroy();
}
  
        Standard_Boolean IsAllocated() const;
  
  Standard_EXPORT    const StepVisual_Array1OfFillStyleSelect& Assign(const StepVisual_Array1OfFillStyleSelect& Other) ;
   const StepVisual_Array1OfFillStyleSelect& operator =(const StepVisual_Array1OfFillStyleSelect& Other) 
{
  return Assign(Other);
}
  
        Standard_Integer Length() const;
  
        Standard_Integer Lower() const;
  
        Standard_Integer Upper() const;
  
        void SetValue(const Standard_Integer Index,const StepVisual_FillStyleSelect& Value) ;
  
       const StepVisual_FillStyleSelect& Value(const Standard_Integer Index) const;
     const StepVisual_FillStyleSelect& operator ()(const Standard_Integer Index) const
{
  return Value(Index);
}
  
        StepVisual_FillStyleSelect& ChangeValue(const Standard_Integer Index) ;
      StepVisual_FillStyleSelect& operator ()(const Standard_Integer Index) 
{
  return ChangeValue(Index);
}





protected:





private:

  
  Standard_EXPORT   StepVisual_Array1OfFillStyleSelect(const StepVisual_Array1OfFillStyleSelect& AnArray);


Standard_Integer myLowerBound;
Standard_Integer myUpperBound;
Standard_Address myStart;
Standard_Boolean isAllocated;


};

#define Array1Item StepVisual_FillStyleSelect
#define Array1Item_hxx <StepVisual_FillStyleSelect.hxx>
#define TCollection_Array1 StepVisual_Array1OfFillStyleSelect
#define TCollection_Array1_hxx <StepVisual_Array1OfFillStyleSelect.hxx>

#include <TCollection_Array1.lxx>

#undef Array1Item
#undef Array1Item_hxx
#undef TCollection_Array1
#undef TCollection_Array1_hxx


// other Inline functions and methods (like "C++: function call" methods)


#endif