// Copyright (c) 2019 Gabriel Cuvillier - Continuation Labs

#ifndef _RWPly_HeaderFile
#define _RWPly_HeaderFile

#include <Message_ProgressIndicator.hxx>
#include <OSD_Path.hxx>
#include <Poly_Triangulation.hxx>
#include <Standard_Macro.hxx>

class RWPly
{
public:

  static Handle(Poly_Triangulation) ReadFile (const Standard_CString theFile,
                                              const Handle(Message_ProgressIndicator)& theProgress);
};

#endif
