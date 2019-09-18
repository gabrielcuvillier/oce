// Created on: 2004-05-18
// Created by: Sergey ZARITCHNY
// Copyright (c) 2004-2014 OPEN CASCADE SAS
//
// Updates by: Copyright (c) 2019 Gabriel Cuvillier - Continuation Labs
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


#include <BinTools.hxx>
#include <BinTools_ShapeSet.hxx>
#include <FSD_FileHeader.hxx>
#include <OSD_OpenFile.hxx>
#include <Storage_StreamTypeMismatchError.hxx>

//=======================================================================
//function : PutBool
//purpose  : 
//=======================================================================
Standard_OStream &BinTools::PutBool(Standard_OStream &OS, const Standard_Boolean aValue) {
  OS.put((Standard_Byte) (aValue ? 1 : 0));
  return OS;
}


//=======================================================================
//function : PutInteger
//purpose  : 
//=======================================================================

Standard_OStream &BinTools::PutInteger(Standard_OStream &OS, const Standard_Integer aValue) {
  Standard_Integer anIntValue = aValue;
#if DO_INVERSE
  anIntValue = InverseInt (aValue);
#endif
  OS.write((char *) &anIntValue, sizeof(Standard_Integer));
  return OS;
}


//=======================================================================
//function : PutReal
//purpose  : 
//=======================================================================

Standard_OStream &BinTools::PutReal(Standard_OStream &OS, const Standard_Real aValue) {
  Standard_Real aRValue = aValue;
#if DO_INVERSE
  aRValue = InverseReal (aValue);
#endif
  OS.write((char *) &aRValue, sizeof(Standard_Real));
  return OS;
}

//=======================================================================
//function : PutShortReal
//purpose  :
//=======================================================================

Standard_OStream &BinTools::PutShortReal(Standard_OStream &OS, const Standard_ShortReal aValue) {
  Standard_ShortReal aRValue = aValue;
#if DO_INVERSE
  aRValue = InverseShortReal (aValue);
#endif
  OS.write((char *) &aRValue, sizeof(Standard_ShortReal));
  return OS;
}

//=======================================================================
//function : PutExtChar
//purpose  : 
//=======================================================================

Standard_OStream &BinTools::PutExtChar(Standard_OStream &OS, const Standard_ExtCharacter aValue) {
  Standard_ExtCharacter aSValue = aValue;
#if DO_INVERSE
  aSValue = InverseExtChar (aValue);
#endif
  OS.write((char *) &aSValue, sizeof(Standard_ExtCharacter));
  return OS;
}

//=======================================================================
//function : GetReal
//purpose  : 
//=======================================================================

Standard_IStream &BinTools::GetReal(Standard_IStream &IS, Standard_Real &aValue) {
  if (!IS.read((char *) &aValue, sizeof(Standard_Real)))
    throw Storage_StreamTypeMismatchError();
#if DO_INVERSE
  aValue = InverseReal (aValue);
#endif
  return IS;
}


//=======================================================================
//function : GetShortReal
//purpose  :
//=======================================================================

Standard_IStream &BinTools::GetShortReal(Standard_IStream &IS, Standard_ShortReal &aValue) {
  if (!IS.read((char *) &aValue, sizeof(Standard_ShortReal)))
    throw Storage_StreamTypeMismatchError();
#if DO_INVERSE
  aValue = InverseShortReal (aValue);
#endif
  return IS;
}


//=======================================================================
//function : GetInteger
//purpose  : 
//=======================================================================

Standard_IStream &BinTools::GetInteger(Standard_IStream &IS, Standard_Integer &aValue) {
  if (!IS.read((char *) &aValue, sizeof(Standard_Integer)))
    throw Storage_StreamTypeMismatchError();;
#if DO_INVERSE
  aValue = InverseInt (aValue);
#endif
  return IS;
}

//=======================================================================
//function : GetExtChar
//purpose  : 
//=======================================================================

Standard_IStream &BinTools::GetExtChar(Standard_IStream &IS, Standard_ExtCharacter &theValue) {
  if (!IS.read((char *) &theValue, sizeof(Standard_ExtCharacter)))
    throw Storage_StreamTypeMismatchError();;
#if DO_INVERSE
  theValue = InverseExtChar (theValue);
#endif
  return IS;
}

//=======================================================================
//function : GetBool
//purpose  : 
//=======================================================================

Standard_IStream &BinTools::GetBool(Standard_IStream &IS, Standard_Boolean &aValue) {
  aValue = (IS.get() != 0);
  return IS;
}

//=======================================================================
//function : Write
//purpose  : 
//=======================================================================

void BinTools::Write(const TopoDS_Shape &theShape, Standard_OStream &theStream) {
  BinTools_ShapeSet aShapeSet(Standard_True);
  aShapeSet.SetFormatNb(3);
  aShapeSet.Add(theShape);
  aShapeSet.Write(theStream);
  aShapeSet.Write(theShape, theStream);
}

//=======================================================================
//function : Read
//purpose  : 
//=======================================================================

void BinTools::Read(TopoDS_Shape &theShape, Standard_IStream &theStream) {
  BinTools_ShapeSet aShapeSet(Standard_True);
  aShapeSet.Read(theStream);
  aShapeSet.Read(theShape, theStream, aShapeSet.NbShapes());
}

//=======================================================================
//function : Write
//purpose  : 
//=======================================================================

Standard_Boolean BinTools::Write(const TopoDS_Shape &theShape, const Standard_CString theFile) {
  ofstream aStream;
  aStream.precision(15);
  OSD_OpenStream(aStream, theFile, ios::out | ios::binary);
  if (!aStream.good())
    return Standard_False;

  Write(theShape, aStream);
  aStream.close();
  return aStream.good();
}

//=======================================================================
//function : Read
//purpose  : 
//=======================================================================

Standard_Boolean BinTools::Read(TopoDS_Shape &theShape, const Standard_CString theFile) {
  filebuf aBuf;
  OSD_OpenStream(aBuf, theFile, ios::in | ios::binary);
  if (!aBuf.is_open())
    return Standard_False;

  Standard_IStream aStream(&aBuf);
  Read(theShape, aStream);
  return aStream.good();
}

//=======================================================================
//function : Write
//purpose  :
//=======================================================================

void BinTools::Write(Handle(Poly_Triangulation) theTri, Standard_OStream &theStream, Standard_Boolean bFormatWithNormals) {
  Standard_Integer j = 0;
  Standard_Integer nbNodes = 0, nbTriangles = 0;

  // write number of nodes
  BinTools::PutInteger(theStream, theTri->NbNodes());
  // write number of triangles
  BinTools::PutInteger(theStream, theTri->NbTriangles());
  // write HasUVNodes
  BinTools::PutBool(theStream, theTri->HasUVNodes() ? 1 : 0);
  if (bFormatWithNormals) {
    // write HasNormals
    BinTools::PutBool(theStream, theTri->HasNormals() ? 1 : 0);
  }
  // write the deflection
  BinTools::PutReal(theStream, theTri->Deflection());

  // write the 3d nodes
  nbNodes = theTri->NbNodes();
  const TColgp_Array1OfPnt &Nodes = theTri->Nodes();
  for (j = 1; j <= nbNodes; j++) {
    BinTools::PutReal(theStream, Nodes(j).X());
    BinTools::PutReal(theStream, Nodes(j).Y());
    BinTools::PutReal(theStream, Nodes(j).Z());
  }

  if (theTri->HasUVNodes()) {
    const TColgp_Array1OfPnt2d &UVNodes = theTri->UVNodes();
    for (j = 1; j <= nbNodes; j++) {
      BinTools::PutReal(theStream, UVNodes(j).X());
      BinTools::PutReal(theStream, UVNodes(j).Y());
    }
  }

  if (bFormatWithNormals) {
    // write HasNormals
    if (theTri->HasNormals()) {
      const TShort_Array1OfShortReal &NormalNodes = theTri->Normals();
      for (j = 1; j <= nbNodes * 3; j++) {
        BinTools::PutShortReal(theStream, NormalNodes(j));
      }
    }
  }

  nbTriangles = theTri->NbTriangles();
  const Poly_Array1OfTriangle &Triangles = theTri->Triangles();
  Standard_Integer n1, n2, n3;
  for (j = 1; j <= nbTriangles; j++) {
    Triangles(j).Get(n1, n2, n3);
    BinTools::PutInteger(theStream, n1);
    BinTools::PutInteger(theStream, n2);
    BinTools::PutInteger(theStream, n3);
  }
}

//=======================================================================
//function : Read
//purpose  :
//=======================================================================

void BinTools::Read(Handle(Poly_Triangulation) &theTri, Standard_IStream &theStream, Standard_Boolean bFormatWithNormals) {
  Standard_Integer j;
  Standard_Real d, x, y, z;
  Standard_Integer nbNodes = 0, nbTriangles = 0;
  Standard_Boolean hasUV = Standard_False;
  Standard_Boolean hasNormals = Standard_False;

  BinTools::GetInteger(theStream, nbNodes);
  BinTools::GetInteger(theStream, nbTriangles);
  TColgp_Array1OfPnt Nodes(1, nbNodes);
  BinTools::GetBool(theStream, hasUV);
  TColgp_Array1OfPnt2d UVNodes(1, nbNodes);
  if (bFormatWithNormals) {
    BinTools::GetBool(theStream, hasNormals);
  }
  TShort_Array1OfShortReal NormalNodes(1, nbNodes * 3);

  BinTools::GetReal(theStream, d); //deflection
  for (j = 1; j <= nbNodes; j++) {
    BinTools::GetReal(theStream, x);
    BinTools::GetReal(theStream, y);
    BinTools::GetReal(theStream, z);
    Nodes(j).SetCoord(x, y, z);
  }

  if (hasUV) {
    for (j = 1; j <= nbNodes; j++) {
      BinTools::GetReal(theStream, x);
      BinTools::GetReal(theStream, y);
      UVNodes(j).SetCoord(x, y);
    }
  }

  if (hasNormals) {
    Standard_ShortReal nc;
    for (j = 1; j <= nbNodes * 3; j++) {
      BinTools::GetShortReal(theStream, nc);
      NormalNodes(j) = nc;
    }
  }

  // read the triangles
  Standard_Integer n1, n2, n3;
  Poly_Array1OfTriangle Triangles(1, nbTriangles);
  for (j = 1; j <= nbTriangles; j++) {
    BinTools::GetInteger(theStream, n1);
    BinTools::GetInteger(theStream, n2);
    BinTools::GetInteger(theStream, n3);
    Triangles(j).Set(n1, n2, n3);
  }

  if (hasUV) {
    if (hasNormals) {
      theTri = new Poly_Triangulation(Nodes, UVNodes, NormalNodes, Triangles);
    } else {
      theTri = new Poly_Triangulation(Nodes, UVNodes, Triangles);
    }
  } else {
    if (hasNormals) {
      theTri = new Poly_Triangulation(Nodes, NormalNodes, Triangles);
    } else {
      theTri = new Poly_Triangulation(Nodes, Triangles);
    }
  }
  theTri->Deflection(d);
}

//=======================================================================
//function : Write
//purpose  :
//=======================================================================

Standard_Boolean BinTools::Write(Handle(Poly_Triangulation) theTri, const Standard_CString theFile) {
  ofstream aStream;
  aStream.precision(15);
  OSD_OpenStream(aStream, theFile, ios::out | ios::binary);
  if (!aStream.good())
    return Standard_False;

  Write(theTri, aStream);
  aStream.close();
  return aStream.good();
}

//=======================================================================
//function : Read
//purpose  :
//=======================================================================

Standard_Boolean BinTools::Read(Handle(Poly_Triangulation) &theTri, const Standard_CString theFile) {
  filebuf aBuf;
  OSD_OpenStream(aBuf, theFile, ios::in | ios::binary);
  if (!aBuf.is_open())
    return Standard_False;

  Standard_IStream aStream(&aBuf);
  Read(theTri, aStream);
  return aStream.good();
}