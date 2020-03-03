// Created by: Kirill GAVRILOV
// Copyright (c) 2013-2014 OPEN CASCADE SAS
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

#include <OpenGl_Texture.hxx>

#include <OpenGl_ArbFBO.hxx>
#include <OpenGl_Context.hxx>
#include <OpenGl_GlCore32.hxx>
#include <OpenGl_Sampler.hxx>
#include <Graphic3d_TextureParams.hxx>
#include <TCollection_ExtendedString.hxx>
#include <Standard_Assert.hxx>
#include <Image_PixMap.hxx>

IMPLEMENT_STANDARD_RTTIEXT(OpenGl_Texture, OpenGl_NamedResource)

//! Simple class to reset unpack alignment settings
struct OpenGl_UnpackAlignmentSentry
{

  //! Reset unpack alignment settings to safe values
  static void Reset()
  {
    glPixelStorei (GL_UNPACK_ALIGNMENT,  1);
  #if !defined(GL_ES_VERSION_2_0)
    glPixelStorei (GL_UNPACK_ROW_LENGTH, 0);
  #endif
  }

  OpenGl_UnpackAlignmentSentry() {}

  ~OpenGl_UnpackAlignmentSentry()
  {
    Reset();
  }

};

// =======================================================================
// function : OpenGl_Texture
// purpose  :
// =======================================================================
OpenGl_Texture::OpenGl_Texture (const TCollection_AsciiString& theResourceId,
                                const Handle(Graphic3d_TextureParams)& theParams)
: OpenGl_NamedResource (theResourceId),
  mySampler (new OpenGl_Sampler (theParams)),
  myRevision (0),
  myTextureId (NO_TEXTURE),
  myTarget (GL_TEXTURE_2D),
  mySizeX (0),
  mySizeY (0),
  mySizeZ (0),
  myTextFormat (GL_RGBA),
  mySizedFormat(GL_RGBA8),
  myNbSamples  (1),
  myHasMipmaps (Standard_False),
  myIsAlpha    (false)
{
  //
}

// =======================================================================
// function : ~OpenGl_Texture
// purpose  :
// =======================================================================
OpenGl_Texture::~OpenGl_Texture()
{
  Release (NULL);
}

// =======================================================================
// function : Create
// purpose  :
// =======================================================================
bool OpenGl_Texture::Create (const Handle(OpenGl_Context)& theCtx)
{
  if (myTextureId != NO_TEXTURE)
  {
    return true;
  }

  theCtx->core11fwd->glGenTextures (1, &myTextureId);
  if (myTextureId == NO_TEXTURE)
  {
    return false;
  }

  //mySampler->Create (theCtx); // do not create sampler object by default
  return true;
}

// =======================================================================
// function : Release
// purpose  :
// =======================================================================
void OpenGl_Texture::Release (OpenGl_Context* theGlCtx)
{
  mySampler->Release (theGlCtx);
  if (myTextureId == NO_TEXTURE)
  {
    return;
  }

  // application can not handle this case by exception - this is bug in code
  Standard_ASSERT_RETURN (theGlCtx != NULL,
    "OpenGl_Texture destroyed without GL context! Possible GPU memory leakage...",);

  if (theGlCtx->IsValid())
  {
    glDeleteTextures (1, &myTextureId);
  }
  myTextureId = NO_TEXTURE;
  mySizeX = mySizeY = mySizeZ = 0;
}

// =======================================================================
// function : applyDefaultSamplerParams
// purpose  :
// =======================================================================
void OpenGl_Texture::applyDefaultSamplerParams (const Handle(OpenGl_Context)& theCtx)
{
  OpenGl_Sampler::applySamplerParams (theCtx, mySampler->Parameters(), NULL, myTarget, myHasMipmaps);
  if (mySampler->IsValid() && !mySampler->IsImmutable())
  {
    OpenGl_Sampler::applySamplerParams (theCtx, mySampler->Parameters(), mySampler.get(), myTarget, myHasMipmaps);
  }
}

// =======================================================================
// function : Bind
// purpose  :
// =======================================================================
void OpenGl_Texture::Bind (const Handle(OpenGl_Context)& theCtx,
                           const Graphic3d_TextureUnit   theTextureUnit) const
{
  if (theCtx->core15fwd != NULL)
  {
    theCtx->core15fwd->glActiveTexture (GL_TEXTURE0 + theTextureUnit);
  }
  mySampler->Bind (theCtx, theTextureUnit);
  glBindTexture (myTarget, myTextureId);
}

// =======================================================================
// function : Unbind
// purpose  :
// =======================================================================
void OpenGl_Texture::Unbind (const Handle(OpenGl_Context)& theCtx,
                             const Graphic3d_TextureUnit   theTextureUnit) const
{
  if (theCtx->core15fwd != NULL)
  {
    theCtx->core15fwd->glActiveTexture (GL_TEXTURE0 + theTextureUnit);
  }
  mySampler->Unbind (theCtx, theTextureUnit);
  glBindTexture (myTarget, NO_TEXTURE);
}

//=======================================================================
//function : InitSamplerObject
//purpose  :
//=======================================================================
bool OpenGl_Texture::InitSamplerObject (const Handle(OpenGl_Context)& theCtx)
{
  return myTextureId != NO_TEXTURE
      && mySampler->Init (theCtx, *this);
}

// =======================================================================
// function : Init
// purpose  :
// =======================================================================
bool OpenGl_Texture::Init (const Handle(OpenGl_Context)& theCtx,
                           const OpenGl_TextureFormat&   theFormat,
                           const Graphic3d_Vec2i&        theSizeXY,
                           const Graphic3d_TypeOfTexture theType,
                           const Image_PixMap*           theImage)
{
  if (theSizeXY.x() < 1
   || theSizeXY.y() < 1)
  {
    theCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH,
                         "Error: texture of 0 size cannot be created.");
    Release (theCtx.get());
    return false;
  }

#if !defined(GL_ES_VERSION_2_0)
  const GLenum aTarget = theType == Graphic3d_TOT_1D
                       ? GL_TEXTURE_1D
                       : GL_TEXTURE_2D;
#else
  const GLenum aTarget = GL_TEXTURE_2D;
#endif
  const Standard_Boolean toCreateMipMaps = (theType == Graphic3d_TOT_2D_MIPMAP);
  const bool toPatchExisting = IsValid()
                            && myTextFormat == theFormat.PixelFormat()
                            && myTarget == aTarget
                            && myHasMipmaps == toCreateMipMaps
                            && mySizeX  == theSizeXY.x()
                            && (mySizeY == theSizeXY.y() || theType == Graphic3d_TOT_1D);
  if (!Create (theCtx))
  {
    Release (theCtx.get());
    return false;
  }

  if (theImage != NULL)
  {
    myIsAlpha = theImage->Format() == Image_Format_Alpha
             || theImage->Format() == Image_Format_AlphaF;
  }
  else
  {
    myIsAlpha = theFormat.PixelFormat() == GL_ALPHA;
  }

  myHasMipmaps             = toCreateMipMaps;
  myTextFormat             = theFormat.PixelFormat();
  mySizedFormat            = theFormat.InternalFormat();
  myNbSamples              = 1;
#if !defined(GL_ES_VERSION_2_0)
  const GLint anIntFormat  = theFormat.InternalFormat();
#else
  // ES 2.0 does not support sized formats and format conversions - them detected from data type
  const GLint anIntFormat  = theCtx->IsGlGreaterEqual (3, 0) ? theFormat.InternalFormat() : theFormat.PixelFormat();
#endif

  if (theFormat.DataType() == GL_FLOAT
  && !theCtx->arbTexFloat)
  {
    theCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH,
                         "Error: floating-point textures are not supported by hardware.");
    Release (theCtx.get());
    return false;
  }

  const GLsizei aMaxSize = theCtx->MaxTextureSize();
  if (theSizeXY.x() > aMaxSize
   || theSizeXY.y() > aMaxSize)
  {
    theCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH,
                         TCollection_AsciiString ("Error: Texture dimension - ") + theSizeXY.x() + "x" + theSizeXY.y()
                       + " exceeds hardware limits (" + aMaxSize + "x" + aMaxSize + ")");
    Release (theCtx.get());
    return false;
  }
#if !defined(GL_ES_VERSION_2_0)
  else if (!theCtx->IsGlGreaterEqual (3, 0) && !theCtx->arbNPTW)
  {
    // Notice that formally general NPOT textures are required by OpenGL 2.0 specifications
    // however some hardware (NV30 - GeForce FX, RadeOn 9xxx and Xxxx) supports GLSL but not NPOT!
    // Trying to create NPOT textures on such hardware will not fail
    // but driver will fall back into software rendering,
    const GLsizei aWidthP2  = OpenGl_Context::GetPowerOfTwo (theSizeXY.x(), aMaxSize);
    const GLsizei aHeightP2 = OpenGl_Context::GetPowerOfTwo (theSizeXY.y(), aMaxSize);
    if (theSizeXY.x() != aWidthP2
     || (theType != Graphic3d_TOT_1D && theSizeXY.y() != aHeightP2))
    {
      theCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_PORTABILITY, 0, GL_DEBUG_SEVERITY_HIGH,
                           TCollection_AsciiString ("Error: NPOT Textures (") + theSizeXY.x() + "x" + theSizeXY.y() + ")"
                           " are not supported by hardware.");
      Release (theCtx.get());
      return false;
    }
  }
#else
  else if (!theCtx->IsGlGreaterEqual (3, 0) && theType == Graphic3d_TOT_2D_MIPMAP)
  {
    // Mipmap NPOT textures are not supported by OpenGL ES 2.0.
    const GLsizei aWidthP2  = OpenGl_Context::GetPowerOfTwo (theSizeXY.x(), aMaxSize);
    const GLsizei aHeightP2 = OpenGl_Context::GetPowerOfTwo (theSizeXY.y(), aMaxSize);
    if (theSizeXY.x() != aWidthP2
     || theSizeXY.y() != aHeightP2)
    {
      theCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_PORTABILITY, 0, GL_DEBUG_SEVERITY_HIGH,
                           TCollection_AsciiString ("Error: Mipmap NPOT Textures (") + theSizeXY.x() + "x" + theSizeXY.y() + ")"
                           " are not supported by OpenGL ES 2.0");
      Release (theCtx.get());
      return false;
    }
  }
#endif

#if !defined(GL_ES_VERSION_2_0)
  GLint aTestWidth  = 0;
  GLint aTestHeight = 0;
#endif
  GLvoid* aDataPtr = (theImage != NULL) ? (GLvoid* )theImage->Data() : NULL;

  // setup the alignment
  OpenGl_UnpackAlignmentSentry anUnpackSentry;
  (void)anUnpackSentry; // avoid compiler warning

  if (aDataPtr != NULL)
  {
    const GLint anAligment = Min ((GLint )theImage->MaxRowAligmentBytes(), 8); // OpenGL supports alignment upto 8 bytes
    glPixelStorei (GL_UNPACK_ALIGNMENT, anAligment);

  #if !defined(GL_ES_VERSION_2_0)
    // notice that GL_UNPACK_ROW_LENGTH is not available on OpenGL ES 2.0 without GL_EXT_unpack_subimage extension
    const GLint anExtraBytes = GLint(theImage->RowExtraBytes());
    const GLint aPixelsWidth = GLint(theImage->SizeRowBytes() / theImage->SizePixelBytes());
    glPixelStorei (GL_UNPACK_ROW_LENGTH, (anExtraBytes >= anAligment) ? aPixelsWidth : 0);
  #endif
  }

  myTarget = aTarget;
  switch (theType)
  {
    case Graphic3d_TOT_1D:
    {
    #if !defined(GL_ES_VERSION_2_0)
      Bind (theCtx);
      applyDefaultSamplerParams (theCtx);
      if (toPatchExisting)
      {
        glTexSubImage1D (GL_TEXTURE_1D, 0, 0,
                         theSizeXY.x(), theFormat.PixelFormat(), theFormat.DataType(), aDataPtr);
        Unbind (theCtx);
        return true;
      }

      // use proxy to check texture could be created or not
      glTexImage1D (GL_PROXY_TEXTURE_1D, 0, anIntFormat,
                    theSizeXY.x(), 0,
                    theFormat.PixelFormat(), theFormat.DataType(), NULL);
      glGetTexLevelParameteriv (GL_PROXY_TEXTURE_1D, 0, GL_TEXTURE_WIDTH, &aTestWidth);
      glGetTexLevelParameteriv (GL_PROXY_TEXTURE_1D, 0, GL_TEXTURE_INTERNAL_FORMAT, &mySizedFormat);
      if (aTestWidth == 0)
      {
        // no memory or broken input parameters
        Unbind (theCtx);
        Release (theCtx.operator->());
        return false;
      }

      glTexImage1D (GL_TEXTURE_1D, 0, anIntFormat,
                    theSizeXY.x(), 0,
                    theFormat.PixelFormat(), theFormat.DataType(), aDataPtr);
      if (glGetError() != GL_NO_ERROR)
      {
        Unbind (theCtx);
        Release (theCtx.get());
        return false;
      }

      mySizeX = theSizeXY.x();
      mySizeY = 1;

      Unbind (theCtx);
      return true;
    #else
      theCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH,
                           "Error: 1D textures are not supported by hardware.");
      Release (theCtx.get());
      return false;
    #endif
    }
    case Graphic3d_TOT_2D:
    {
      Bind (theCtx);
      applyDefaultSamplerParams (theCtx);
      if (toPatchExisting)
      {
        glTexSubImage2D (GL_TEXTURE_2D, 0,
                         0, 0,
                         theSizeXY.x(), theSizeXY.y(),
                         theFormat.PixelFormat(), theFormat.DataType(), aDataPtr);
        Unbind (theCtx);
        return true;
      }

    #if !defined(GL_ES_VERSION_2_0)
      // use proxy to check texture could be created or not
      glTexImage2D (GL_PROXY_TEXTURE_2D, 0, anIntFormat,
                    theSizeXY.x(), theSizeXY.y(), 0,
                    theFormat.PixelFormat(), theFormat.DataType(), NULL);
      glGetTexLevelParameteriv (GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &aTestWidth);
      glGetTexLevelParameteriv (GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &aTestHeight);
      glGetTexLevelParameteriv (GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &mySizedFormat);
      if (aTestWidth == 0 || aTestHeight == 0)
      {
        // no memory or broken input parameters
        Unbind (theCtx);
        Release (theCtx.get());
        return false;
      }
    #endif

      glTexImage2D (GL_TEXTURE_2D, 0, anIntFormat,
                    theSizeXY.x(), theSizeXY.y(), 0,
                    theFormat.PixelFormat(), theFormat.DataType(), aDataPtr);
      const GLenum anErr = glGetError();
      if (anErr != GL_NO_ERROR)
      {
        theCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH,
                             TCollection_AsciiString ("Error: 2D texture ") + theSizeXY.x() + "x" + theSizeXY.y()
                                                   + " IF: " + int(anIntFormat) + " PF: " + int(theFormat.PixelFormat())
                                                   + " DT: " + int(theFormat.DataType())
                                                   + " can not be created with error " + int(anErr) + ".");
        Unbind (theCtx);
        Release (theCtx.get());
        return false;
      }

      mySizeX = theSizeXY.x();
      mySizeY = theSizeXY.y();

      Unbind (theCtx);
      return true;
    }
    case Graphic3d_TOT_2D_MIPMAP:
    {
      Bind (theCtx);
      applyDefaultSamplerParams (theCtx);
      if (toPatchExisting)
      {
        glTexSubImage2D (GL_TEXTURE_2D, 0,
                         0, 0,
                         theSizeXY.x(), theSizeXY.y(),
                         theFormat.PixelFormat(), theFormat.DataType(), aDataPtr);
        if (theCtx->arbFBO != NULL)
        {
          // generate mipmaps
          theCtx->arbFBO->glGenerateMipmap (GL_TEXTURE_2D);
          if (glGetError() != GL_NO_ERROR)
          {
            Unbind (theCtx);
            Release (theCtx.get());
            return false;
          }
        }

        Unbind (theCtx);
        return true;
      }

    #if !defined(GL_ES_VERSION_2_0)
      // use proxy to check texture could be created or not
      glTexImage2D (GL_PROXY_TEXTURE_2D, 0, anIntFormat,
                    theSizeXY.x(), theSizeXY.y(), 0,
                    theFormat.PixelFormat(), theFormat.DataType(), NULL);
      glGetTexLevelParameteriv (GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &aTestWidth);
      glGetTexLevelParameteriv (GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &aTestHeight);
      glGetTexLevelParameteriv (GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &mySizedFormat);
      if (aTestWidth == 0 || aTestHeight == 0)
      {
        // no memory or broken input parameters
        Unbind (theCtx);
        Release (theCtx.get());
        return false;
      }
    #endif

      // upload main picture
      glTexImage2D (GL_TEXTURE_2D, 0, anIntFormat,
                    theSizeXY.x(), theSizeXY.y(), 0,
                    theFormat.PixelFormat(), theFormat.DataType(), theImage->Data());
      const GLenum aTexImgErr = glGetError();
      if (aTexImgErr != GL_NO_ERROR)
      {
        theCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH,
                             TCollection_AsciiString ("Error: 2D texture ") + theSizeXY.x() + "x" + theSizeXY.y()
                                                    + " IF: " + int(anIntFormat) + " PF: " + int(theFormat.PixelFormat())
                                                    + " DT: " + int(theFormat.DataType())
                                                    + " can not be created with error " + int(aTexImgErr) + ".");
        Unbind (theCtx);
        Release (theCtx.get());
        return false;
      }

      mySizeX = theSizeXY.x();
      mySizeY = theSizeXY.y();

      if (theCtx->arbFBO != NULL)
      {
        // generate mipmaps
        //glHint (GL_GENERATE_MIPMAP_HINT, GL_NICEST);
        theCtx->arbFBO->glGenerateMipmap (GL_TEXTURE_2D);

        if (glGetError() != GL_NO_ERROR)
        {
          Unbind (theCtx);
          Release (theCtx.get());
          return false;
        }
      }
      else
      {
        theCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_PORTABILITY, 0, GL_DEBUG_SEVERITY_HIGH,
                             "Warning: generating mipmaps requires GL_ARB_framebuffer_object extension which is missing.");
        Unbind (theCtx);
        Release (theCtx.get());
        return false;
      }

      Unbind (theCtx);
      return true;
    }
    case Graphic3d_TOT_CUBEMAP:
    {
      Unbind (theCtx);
      Release (theCtx.get());
      return false;
    }
  }

  Release (theCtx.get());
  return false;
}

// =======================================================================
// function : Init
// purpose  :
// =======================================================================
bool OpenGl_Texture::Init (const Handle(OpenGl_Context)& theCtx,
                           const Image_PixMap&           theImage,
                           const Graphic3d_TypeOfTexture theType,
                           const Standard_Boolean        theIsColorMap)
{
  if (theImage.IsEmpty())
  {
    Release (theCtx.get());
    return false;
  }

  const OpenGl_TextureFormat aFormat = OpenGl_TextureFormat::FindFormat (theCtx, theImage.Format(), theIsColorMap);
  if (!aFormat.IsValid())
  {
    Release (theCtx.get());
    return false;
  }

  return Init (theCtx, aFormat, Graphic3d_Vec2i ((Standard_Integer)theImage.SizeX(), (Standard_Integer)theImage.SizeY()),
               theType, &theImage);
}

// =======================================================================
// function : Init
// purpose  :
// =======================================================================
bool OpenGl_Texture::Init (const Handle(OpenGl_Context)&       theCtx,
                           const Handle(Graphic3d_TextureMap)& theTextureMap)
{
  if (theTextureMap.IsNull())
  {
    return false;
  }

  switch (theTextureMap->Type())
  {
    case Graphic3d_TOT_CUBEMAP:
    {
      return InitCubeMap (theCtx, Handle(Graphic3d_CubeMap)::DownCast(theTextureMap),
                          0, Image_Format_RGB, false, theTextureMap->IsColorMap());
    }
    default:
    {
      Handle(Image_PixMap) anImage = theTextureMap->GetImage();
      if (anImage.IsNull())
      {
        return false;
      }
      return Init (theCtx, *anImage, theTextureMap->Type(), theTextureMap->IsColorMap());
    }
  }
}

// =======================================================================
// function : Init2DMultisample
// purpose  :
// =======================================================================
bool OpenGl_Texture::Init2DMultisample (const Handle(OpenGl_Context)& theCtx,
                                        const GLsizei                 theNbSamples,
                                        const GLint                   theTextFormat,
                                        const GLsizei                 theSizeX,
                                        const GLsizei                 theSizeY)
{
  if (!Create (theCtx)
    || theNbSamples > theCtx->MaxMsaaSamples()
    || theNbSamples < 1)
  {
    return false;
  }

  myNbSamples = OpenGl_Context::GetPowerOfTwo (theNbSamples, theCtx->MaxMsaaSamples());
  myTarget = GL_TEXTURE_2D_MULTISAMPLE;
  myHasMipmaps = false;
  if(theSizeX > theCtx->MaxTextureSize()
  || theSizeY > theCtx->MaxTextureSize())
  {
    return false;
  }

  Bind (theCtx);
  //myTextFormat = theTextFormat;
  mySizedFormat = theTextFormat;
#if !defined(GL_ES_VERSION_2_0)
  if (theCtx->Functions()->glTexStorage2DMultisample != NULL)
  {
    theCtx->Functions()->glTexStorage2DMultisample (myTarget, myNbSamples, theTextFormat, theSizeX, theSizeY, GL_FALSE);
  }
  else
  {
    theCtx->Functions()->glTexImage2DMultisample   (myTarget, myNbSamples, theTextFormat, theSizeX, theSizeY, GL_FALSE);
  }
#else
#if !defined(HAVE_WEBGL)
  theCtx->Functions()  ->glTexStorage2DMultisample (myTarget, myNbSamples, theTextFormat, theSizeX, theSizeY, GL_FALSE);
#else
  Unbind (theCtx);
  return false;
#endif
#endif
  if (theCtx->core11fwd->glGetError() != GL_NO_ERROR)
  {
    Unbind (theCtx);
    return false;
  }

  mySizeX = theSizeX;
  mySizeY = theSizeY;

  Unbind (theCtx);
  return true;
}

// =======================================================================
// function : InitRectangle
// purpose  :
// =======================================================================
bool OpenGl_Texture::InitRectangle (const Handle(OpenGl_Context)& theCtx,
                                    const Standard_Integer        theSizeX,
                                    const Standard_Integer        theSizeY,
                                    const OpenGl_TextureFormat&   theFormat)
{
  if (!Create (theCtx) || !theCtx->IsGlGreaterEqual (3, 0))
  {
    return false;
  }

#if !defined(GL_ES_VERSION_2_0)
  myTarget = GL_TEXTURE_RECTANGLE;
  myNbSamples = 1;
  myHasMipmaps = false;

  const GLsizei aSizeX    = Min (theCtx->MaxTextureSize(), theSizeX);
  const GLsizei aSizeY    = Min (theCtx->MaxTextureSize(), theSizeY);

  Bind (theCtx);
  applyDefaultSamplerParams (theCtx);

  myTextFormat  = theFormat.Format();
  mySizedFormat = theFormat.Internal();

  // setup the alignment
  OpenGl_UnpackAlignmentSentry::Reset();

  glTexImage2D (GL_PROXY_TEXTURE_RECTANGLE, 0, mySizedFormat,
                aSizeX, aSizeY, 0,
                myTextFormat, GL_FLOAT, NULL);

  GLint aTestSizeX = 0;
  GLint aTestSizeY = 0;

  glGetTexLevelParameteriv (GL_PROXY_TEXTURE_RECTANGLE, 0, GL_TEXTURE_WIDTH,  &aTestSizeX);
  glGetTexLevelParameteriv (GL_PROXY_TEXTURE_RECTANGLE, 0, GL_TEXTURE_HEIGHT, &aTestSizeY);
  glGetTexLevelParameteriv (GL_PROXY_TEXTURE_RECTANGLE, 0, GL_TEXTURE_INTERNAL_FORMAT, &mySizedFormat);

  if (aTestSizeX == 0 || aTestSizeY == 0)
  {
    Unbind (theCtx);
    return false;
  }

  glTexImage2D (myTarget, 0, mySizedFormat,
                aSizeX, aSizeY, 0,
                myTextFormat, GL_FLOAT, NULL);

  if (glGetError() != GL_NO_ERROR)
  {
    Unbind (theCtx);
    return false;
  }

  mySizeX = aSizeX;
  mySizeY = aSizeY;

  Unbind (theCtx);
  return true;
#else
  (void )theSizeX;
  (void )theSizeY;
  (void )theFormat;
  return false;
#endif
}

// =======================================================================
// function : Init3D
// purpose  :
// =======================================================================
bool OpenGl_Texture::Init3D (const Handle(OpenGl_Context)& theCtx,
                             const OpenGl_TextureFormat&   theFormat,
                             const Graphic3d_Vec3i&        theSizeXYZ,
                             const void*                   thePixels)
{
  if (theCtx->Functions()->glTexImage3D == NULL)
  {
    theCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH,
                         "Error: three-dimensional textures are not supported by hardware.");
    return false;
  }

  if (!Create(theCtx))
  {
    return false;
  }

  myTarget = GL_TEXTURE_3D;
  myNbSamples = 1;
  myHasMipmaps = false;

  const Graphic3d_Vec3i aSizeXYZ = theSizeXYZ.cwiseMin (Graphic3d_Vec3i (theCtx->MaxTextureSize()));
  if (aSizeXYZ != theSizeXYZ)
  {
    theCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH,
                         "Error: 3D texture dimensions exceed hardware limits.");
    Release (theCtx.get());
    Unbind (theCtx);
    return false;
  }
  Bind (theCtx);

  if (theFormat.DataType() == GL_FLOAT
  && !theCtx->arbTexFloat)
  {
    theCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH,
                         "Error: floating-point textures are not supported by hardware.");
    Release (theCtx.get());
    Unbind (theCtx);
    return false;
  }

  mySizedFormat = theFormat.InternalFormat();

  // setup the alignment
  OpenGl_UnpackAlignmentSentry::Reset();

#if !defined (GL_ES_VERSION_2_0)
  theCtx->core15fwd->glTexImage3D (GL_PROXY_TEXTURE_3D, 0, mySizedFormat,
                                   aSizeXYZ.x(), aSizeXYZ.y(), aSizeXYZ.z(), 0,
                                   theFormat.PixelFormat(), theFormat.DataType(), NULL);

  NCollection_Vec3<GLint> aTestSizeXYZ;
  glGetTexLevelParameteriv (GL_PROXY_TEXTURE_3D, 0, GL_TEXTURE_WIDTH,  &aTestSizeXYZ.x());
  glGetTexLevelParameteriv (GL_PROXY_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &aTestSizeXYZ.y());
  glGetTexLevelParameteriv (GL_PROXY_TEXTURE_3D, 0, GL_TEXTURE_DEPTH,  &aTestSizeXYZ.z());
  glGetTexLevelParameteriv (GL_PROXY_TEXTURE_3D, 0, GL_TEXTURE_INTERNAL_FORMAT, &mySizedFormat);
  if (aTestSizeXYZ.x() == 0 || aTestSizeXYZ.y() == 0 || aTestSizeXYZ.z() == 0)
  {
    Unbind (theCtx);
    Release (theCtx.get());
    return false;
  }
#endif

  applyDefaultSamplerParams (theCtx);
  theCtx->Functions()->glTexImage3D (myTarget, 0, mySizedFormat,
                                     aSizeXYZ.x(), aSizeXYZ.y(), aSizeXYZ.z(), 0,
                                     theFormat.PixelFormat(), theFormat.DataType(), thePixels);

  if (glGetError() != GL_NO_ERROR)
  {
    Unbind (theCtx);
    Release (theCtx.get());
    return false;
  }

  mySizeX = aSizeXYZ.x();
  mySizeY = aSizeXYZ.y();
  mySizeZ = aSizeXYZ.z();

  Unbind (theCtx);
  return true;
}

// =======================================================================
// function : InitCubeMap
// purpose  :
// =======================================================================
bool OpenGl_Texture::InitCubeMap (const Handle(OpenGl_Context)&    theCtx,
                                  const Handle(Graphic3d_CubeMap)& theCubeMap,
                                  Standard_Size    theSize,
                                  Image_Format     theFormat,
                                  Standard_Boolean theToGenMipmap,
                                  Standard_Boolean theIsColorMap)
{
  if (!Create (theCtx))
  {
    Release (theCtx.get());
    return false;
  }

  if (!theCubeMap.IsNull())
  {
    theToGenMipmap = theCubeMap->HasMipmaps();
    if (Handle(Image_PixMap) anImage = theCubeMap->Reset().Value())
    {
      theSize   = anImage->SizeX();
      theFormat = anImage->Format();
    }
    else
    {
      theCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH,
                           "Unable to get the first side of cubemap");
      Release(theCtx.get());
      return false;
    }
  }

  OpenGl_TextureFormat aFormat = OpenGl_TextureFormat::FindFormat (theCtx, theFormat, theIsColorMap);
  if (!aFormat.IsValid())
  {
    Unbind(theCtx);
    Release(theCtx.get());
    return false;
  }

  myTarget = GL_TEXTURE_CUBE_MAP;
  myHasMipmaps = theToGenMipmap;
  myNbSamples = 1;
  mySizeX = (GLsizei )theSize;
  mySizeY = (GLsizei )theSize;
  Bind (theCtx);
  applyDefaultSamplerParams (theCtx);

  for (Standard_Integer i = 0; i < 6; ++i)
  {
    const void* aData = NULL;
    Handle(Image_PixMap) anImage;

    if (!theCubeMap.IsNull())
    {
      anImage = theCubeMap->Value();
      if (!anImage.IsNull())
      {
#if !defined(GL_ES_VERSION_2_0)
        const GLint anAligment = Min ((GLint)anImage->MaxRowAligmentBytes(), 8); // OpenGL supports alignment upto 8 bytes
        glPixelStorei (GL_UNPACK_ALIGNMENT, anAligment);

        // notice that GL_UNPACK_ROW_LENGTH is not available on OpenGL ES 2.0 without GL_EXT_unpack_subimage extension
        const GLint anExtraBytes = GLint(anImage->RowExtraBytes());
        const GLint aPixelsWidth = GLint(anImage->SizeRowBytes() / anImage->SizePixelBytes());
        const GLint aRowLength = (anExtraBytes >= anAligment) ? aPixelsWidth : 0;
        glPixelStorei (GL_UNPACK_ROW_LENGTH, aRowLength);
#else
        Handle(Image_PixMap) aCopyImage = new Image_PixMap();
        aCopyImage->InitTrash (theFormat, theSize, theSize);
        for (unsigned int y = 0; y < theSize; ++y)
        {
          for (unsigned int x = 0; x < theSize; ++x)
          {
            for (unsigned int aByte = 0; aByte < anImage->SizePixelBytes(); ++aByte)
            {
              aCopyImage->ChangeRawValue (y, x)[aByte] = anImage->RawValue (y, x)[aByte];
            }
          }
        }
        anImage = aCopyImage;
        const GLint anAligment = Min((GLint)anImage->MaxRowAligmentBytes(), 8); // OpenGL supports alignment upto 8 bytes
        glPixelStorei (GL_UNPACK_ALIGNMENT, anAligment);
#endif
        aData = anImage->Data();
      }
      else
      {
        theCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH,
                             TCollection_AsciiString() + "Unable to get [" + i + "] side of cubemap");
        Unbind (theCtx);
        Release (theCtx.get());
        return false;
      }
      theCubeMap->Next();
    }

    glTexImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                  aFormat.InternalFormat(),
                  GLsizei(theSize), GLsizei(theSize),
                  0, aFormat.PixelFormat(), aFormat.DataType(),
                  aData);

    OpenGl_UnpackAlignmentSentry::Reset();

    const GLenum anErr = glGetError();
    if (anErr != GL_NO_ERROR)
    {
      theCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH,
                           TCollection_AsciiString ("Unable to initialize side of cubemap. Error #") + int(anErr));
      Unbind (theCtx);
      Release (theCtx.get());
      return false;
    }
  }

  if (theToGenMipmap && theCtx->arbFBO != NULL)
  {
    theCtx->arbFBO->glGenerateMipmap (myTarget);
    const GLenum anErr = glGetError();
    if (anErr != GL_NO_ERROR)
    {
      theCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH,
                           TCollection_AsciiString ("Unable to generate mipmap of cubemap. Error #") + int(anErr));
      Unbind (theCtx);
      Release (theCtx.get());
      return false;
    }
  }

  Unbind (theCtx.get());
  return true;
}

// =======================================================================
// function : PixelSizeOfPixelFormat
// purpose  :
// =======================================================================
Standard_Size OpenGl_Texture::PixelSizeOfPixelFormat (Standard_Integer theInternalFormat)
{
  switch(theInternalFormat)
  {
    // RED variations (GL_RED, OpenGL 3.0+)
    case GL_RED:
    case GL_R8:       return 1;
    case GL_R16:      return 2;
    case GL_R16F:     return 2;
    case GL_R32F:     return 4;
    // RGB variations
    case GL_RGB:      return 3;
    case GL_RGB8:     return 3;
    case GL_RGB16:    return 6;
    case GL_RGB16F:   return 6;
    case GL_RGB32F:   return 12;
    // RGBA variations
    case GL_RGBA:     return 4;
    case GL_RGBA8:    return 4;
    case GL_RGB10_A2: return 4;
    case GL_RGBA12:   return 6;
    case GL_RGBA16:   return 8;
    case GL_RGBA16F:  return 8;
    case GL_RGBA32F:  return 16;
    //
    case GL_BGRA_EXT:  return 4;
    // ALPHA variations (deprecated)
    case GL_ALPHA:
    case GL_ALPHA8:    return 1;
    case GL_ALPHA16:   return 2;
    case GL_LUMINANCE: return 1;
    case GL_LUMINANCE_ALPHA: return 2;
    // depth-stencil
    case GL_DEPTH24_STENCIL8:   return 4;
    case GL_DEPTH32F_STENCIL8:  return 8;
    case GL_DEPTH_COMPONENT16:  return 2;
    case GL_DEPTH_COMPONENT24:  return 3;
    case GL_DEPTH_COMPONENT32F: return 4;
  }
  return 0;
}

// =======================================================================
// function : EstimatedDataSize
// purpose  :
// =======================================================================
Standard_Size OpenGl_Texture::EstimatedDataSize() const
{
  if (!IsValid())
  {
    return 0;
  }

  Standard_Size aSize = PixelSizeOfPixelFormat (mySizedFormat) * mySizeX * myNbSamples;
  if (mySizeY != 0)
  {
    aSize *= Standard_Size(mySizeY);
  }
  if (mySizeZ != 0)
  {
    aSize *= Standard_Size(mySizeZ);
  }
  if (myTarget == GL_TEXTURE_CUBE_MAP)
  {
    aSize *= 6; // cube sides
  }
  if (myHasMipmaps)
  {
    aSize = aSize + aSize / 3;
  }
  return aSize;
}
