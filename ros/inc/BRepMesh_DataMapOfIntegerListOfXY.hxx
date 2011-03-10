// This file is generated by WOK (CPPExt).
// Please do not edit this file; modify original file instead.
// The copyright and license terms as defined for the original file apply to 
// this header file considered to be the "object code" form of the original source.

#ifndef _BRepMesh_DataMapOfIntegerListOfXY_HeaderFile
#define _BRepMesh_DataMapOfIntegerListOfXY_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif

#ifndef _TCollection_BasicMap_HeaderFile
#include <TCollection_BasicMap.hxx>
#endif
#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
#ifndef _Handle_BRepMesh_DataMapNodeOfDataMapOfIntegerListOfXY_HeaderFile
#include <Handle_BRepMesh_DataMapNodeOfDataMapOfIntegerListOfXY.hxx>
#endif
#ifndef _Standard_Boolean_HeaderFile
#include <Standard_Boolean.hxx>
#endif
class Standard_DomainError;
class Standard_NoSuchObject;
class BRepMesh_ListOfXY;
class TColStd_MapIntegerHasher;
class BRepMesh_DataMapNodeOfDataMapOfIntegerListOfXY;
class BRepMesh_DataMapIteratorOfDataMapOfIntegerListOfXY;



class BRepMesh_DataMapOfIntegerListOfXY  : public TCollection_BasicMap {
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

  
  Standard_EXPORT   BRepMesh_DataMapOfIntegerListOfXY(const Standard_Integer NbBuckets = 1);
  
  Standard_EXPORT     BRepMesh_DataMapOfIntegerListOfXY& Assign(const BRepMesh_DataMapOfIntegerListOfXY& Other) ;
    BRepMesh_DataMapOfIntegerListOfXY& operator =(const BRepMesh_DataMapOfIntegerListOfXY& Other) 
{
  return Assign(Other);
}
  
  Standard_EXPORT     void ReSize(const Standard_Integer NbBuckets) ;
  
  Standard_EXPORT     void Clear() ;
~BRepMesh_DataMapOfIntegerListOfXY()
{
  Clear();
}
  
  Standard_EXPORT     Standard_Boolean Bind(const Standard_Integer& K,const BRepMesh_ListOfXY& I) ;
  
  Standard_EXPORT     Standard_Boolean IsBound(const Standard_Integer& K) const;
  
  Standard_EXPORT     Standard_Boolean UnBind(const Standard_Integer& K) ;
  
  Standard_EXPORT    const BRepMesh_ListOfXY& Find(const Standard_Integer& K) const;
   const BRepMesh_ListOfXY& operator()(const Standard_Integer& K) const
{
  return Find(K);
}
  
  Standard_EXPORT     BRepMesh_ListOfXY& ChangeFind(const Standard_Integer& K) ;
    BRepMesh_ListOfXY& operator()(const Standard_Integer& K) 
{
  return ChangeFind(K);
}





protected:





private:

  
  Standard_EXPORT   BRepMesh_DataMapOfIntegerListOfXY(const BRepMesh_DataMapOfIntegerListOfXY& Other);




};





// other Inline functions and methods (like "C++: function call" methods)


#endif