// Created on: 2006-04-13
// Created by: Andrey BETENEV
// Copyright (c) 2006-2014 OPEN CASCADE SAS
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

#include <Standard_Mutex.hxx>
#include <Standard_OStream.hxx>

#include <errno.h>

namespace {
const Standard_Boolean ToUseThreads =
#if !defined(OCCT_DISABLE_THREADS)
  Standard_True;
#else
  Standard_False;
#endif
}

//=============================================
// Standard_Mutex::Standard_Mutex
//=============================================

Standard_Mutex::Standard_Mutex () 
{
#if (defined(_WIN32) || defined(__WIN32__))
  InitializeCriticalSection (&myMutex);
#else
  if Standard_IF_CONSTEXPR(ToUseThreads) {
    pthread_mutexattr_t anAttr;
    pthread_mutexattr_init(&anAttr);
    pthread_mutexattr_settype(&anAttr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&myMutex, &anAttr);
    pthread_mutexattr_destroy(&anAttr);
  }
#endif
}

//=============================================
// Standard_Mutex::~Standard_Mutex
//=============================================

Standard_Mutex::~Standard_Mutex () 
{
#if (defined(_WIN32) || defined(__WIN32__))
  DeleteCriticalSection (&myMutex);
#else
  if Standard_IF_CONSTEXPR(ToUseThreads) {
    pthread_mutex_destroy(&myMutex);
  }
#endif
}

//=============================================
// Standard_Mutex::Lock
//=============================================

void Standard_Mutex::Lock ()
{
#if (defined(_WIN32) || defined(__WIN32__))
  EnterCriticalSection (&myMutex);
#else
  if Standard_IF_CONSTEXPR(ToUseThreads) {
    pthread_mutex_lock(&myMutex);
  }
#endif
}

//=============================================
// Standard_Mutex::TryLock
//=============================================

Standard_Boolean Standard_Mutex::TryLock ()
{
#if (defined(_WIN32) || defined(__WIN32__))
  return (TryEnterCriticalSection (&myMutex) != 0);
#else
  if Standard_IF_CONSTEXPR(ToUseThreads) {
    return (pthread_mutex_trylock(&myMutex) != EBUSY);
  }
  else {
    return Standard_True;
  }
#endif
}

//=============================================
// Standard_Mutex::DestroyCallback
//=============================================

void Standard_Mutex::DestroyCallback ()
{
  UnregisterCallback();
  Unlock();
}

//=============================================
// Standard_Mutex::Unlock
//=============================================

void Standard_Mutex::Unlock ()
{
#if (defined(_WIN32) || defined(__WIN32__))
  LeaveCriticalSection (&myMutex);
#else
  if Standard_IF_CONSTEXPR(ToUseThreads) {
    pthread_mutex_unlock (&myMutex);
  }
#endif
}
