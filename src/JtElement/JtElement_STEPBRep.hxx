
#ifndef _JtElement_STEPBRep_HeaderFile
#define _JtElement_STEPBRep_HeaderFile

#include <JtData_Object.hxx>

class JtElement_STEPBRep : public JtData_Object
{
public:
  //! Default constructor.
  JtElement_STEPBRep() {}

  //! Read this entity from a JT file.
  Standard_EXPORT virtual Standard_Boolean Read (JtData_Reader &theReader) Standard_OVERRIDE;

  //! Dump this entity.
  Standard_EXPORT virtual Standard_Integer Dump (Standard_OStream& S) const Standard_OVERRIDE;

  const Jt_String& Data() const { return myData; }

  void ClearData() { myData.Free(); }

  DEFINE_STANDARD_RTTIEXT(JtElement_STEPBRep, JtData_Object)
  DEFINE_OBJECT_CLASS (JtElement_STEPBRep)

protected:
  Jt_String myData;        //!< STEP Data
};

DEFINE_STANDARD_HANDLE(JtElement_STEPBRep, JtData_Object)

#endif // _JtElement_STEPBRep_HeaderFile
