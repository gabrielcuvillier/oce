// Copyright (c) 2019 Gabriel Cuvillier - Continuation Labs

#if defined(HAVE_TINYPLY)
// Be sure to include these files before tinyply header, due to possible throw/try/catch and constexpr being redefined
#include <Standard_Macro.hxx>
#include <Standard_DefineException.hxx>

#define TINYPLY_IMPLEMENTATION
#include <tinyply/tinyply.h>
#endif
