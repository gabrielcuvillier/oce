
#include <JtElement_STEPBRep.hxx>
#include <JtData_Reader.hxx>

#include <TColStd_HArray1OfCharacter.hxx>

IMPLEMENT_STANDARD_HANDLE (JtElement_STEPBRep, JtData_Object)
IMPLEMENT_STANDARD_RTTIEXT(JtElement_STEPBRep, JtData_Object)

IMPLEMENT_OBJECT_CLASS(JtElement_STEPBRep, "STEP B-Rep Object",
                       "869c7d53-ccb0-451b-b2-03-d1-42-81-0x56-14-56")

//=======================================================================
//function : Read
//purpose  : Read this entity from a translate file
//=======================================================================
Standard_Boolean JtElement_STEPBRep::Read (JtData_Reader& theReader)
{
  if (!JtData_Object::Read (theReader))
  {
    return Standard_False;
  }

  return theReader.ReadVec (myData);
}

//=======================================================================
//function : Dump
//purpose  :
//=======================================================================
Standard_Integer JtElement_STEPBRep::Dump (Standard_OStream& theStream) const
{
  theStream << "JtElement_STEPBRep { " << " } ";
  return JtData_Object::Dump (theStream);
}
