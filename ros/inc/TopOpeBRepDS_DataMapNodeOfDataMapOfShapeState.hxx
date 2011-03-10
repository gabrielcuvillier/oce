// This file is generated by WOK (CPPExt).
// Please do not edit this file; modify original file instead.
// The copyright and license terms as defined for the original file apply to 
// this header file considered to be the "object code" form of the original source.

#ifndef _TopOpeBRepDS_DataMapNodeOfDataMapOfShapeState_HeaderFile
#define _TopOpeBRepDS_DataMapNodeOfDataMapOfShapeState_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Standard_DefineHandle_HeaderFile
#include <Standard_DefineHandle.hxx>
#endif
#ifndef _Handle_TopOpeBRepDS_DataMapNodeOfDataMapOfShapeState_HeaderFile
#include <Handle_TopOpeBRepDS_DataMapNodeOfDataMapOfShapeState.hxx>
#endif

#ifndef _TopoDS_Shape_HeaderFile
#include <TopoDS_Shape.hxx>
#endif
#ifndef _TopAbs_State_HeaderFile
#include <TopAbs_State.hxx>
#endif
#ifndef _TCollection_MapNode_HeaderFile
#include <TCollection_MapNode.hxx>
#endif
#ifndef _TCollection_MapNodePtr_HeaderFile
#include <TCollection_MapNodePtr.hxx>
#endif
class TopoDS_Shape;
class TopTools_ShapeMapHasher;
class TopOpeBRepDS_DataMapOfShapeState;
class TopOpeBRepDS_DataMapIteratorOfDataMapOfShapeState;



class TopOpeBRepDS_DataMapNodeOfDataMapOfShapeState : public TCollection_MapNode {

public:

  
      TopOpeBRepDS_DataMapNodeOfDataMapOfShapeState(const TopoDS_Shape& K,const TopAbs_State& I,const TCollection_MapNodePtr& n);
  
        TopoDS_Shape& Key() const;
  
        TopAbs_State& Value() const;




  DEFINE_STANDARD_RTTI(TopOpeBRepDS_DataMapNodeOfDataMapOfShapeState)

protected:




private: 


TopoDS_Shape myKey;
TopAbs_State myValue;


};

#define TheKey TopoDS_Shape
#define TheKey_hxx <TopoDS_Shape.hxx>
#define TheItem TopAbs_State
#define TheItem_hxx <TopAbs_State.hxx>
#define Hasher TopTools_ShapeMapHasher
#define Hasher_hxx <TopTools_ShapeMapHasher.hxx>
#define TCollection_DataMapNode TopOpeBRepDS_DataMapNodeOfDataMapOfShapeState
#define TCollection_DataMapNode_hxx <TopOpeBRepDS_DataMapNodeOfDataMapOfShapeState.hxx>
#define TCollection_DataMapIterator TopOpeBRepDS_DataMapIteratorOfDataMapOfShapeState
#define TCollection_DataMapIterator_hxx <TopOpeBRepDS_DataMapIteratorOfDataMapOfShapeState.hxx>
#define Handle_TCollection_DataMapNode Handle_TopOpeBRepDS_DataMapNodeOfDataMapOfShapeState
#define TCollection_DataMapNode_Type_() TopOpeBRepDS_DataMapNodeOfDataMapOfShapeState_Type_()
#define TCollection_DataMap TopOpeBRepDS_DataMapOfShapeState
#define TCollection_DataMap_hxx <TopOpeBRepDS_DataMapOfShapeState.hxx>

#include <TCollection_DataMapNode.lxx>

#undef TheKey
#undef TheKey_hxx
#undef TheItem
#undef TheItem_hxx
#undef Hasher
#undef Hasher_hxx
#undef TCollection_DataMapNode
#undef TCollection_DataMapNode_hxx
#undef TCollection_DataMapIterator
#undef TCollection_DataMapIterator_hxx
#undef Handle_TCollection_DataMapNode
#undef TCollection_DataMapNode_Type_
#undef TCollection_DataMap
#undef TCollection_DataMap_hxx


// other Inline functions and methods (like "C++: function call" methods)


#endif