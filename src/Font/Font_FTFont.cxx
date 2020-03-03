// Created on: 2013-01-28
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

#include <Font_FTFont.hxx>

#include <Font_FTLibrary.hxx>
#include <Font_FontMgr.hxx>
#include <Font_TextFormatter.hxx>
#include <Message.hxx>
#include <Message_Messenger.hxx>

#include "Font_DejavuSans_Latin_woff.pxx"

#include <algorithm>

#include <ft2build.h>
#include FT_FREETYPE_H

IMPLEMENT_STANDARD_RTTIEXT(Font_FTFont,Standard_Transient)

// =======================================================================
// function : Font_FTFont
// purpose  :
// =======================================================================
Font_FTFont::Font_FTFont (const Handle(Font_FTLibrary)& theFTLib)
: myFTLib       (theFTLib),
  myFTFace      (NULL),
  myActiveFTFace(NULL),
  myFontAspect  (Font_FontAspect_Regular),
  myWidthScaling(1.0),
  myLoadFlags   (FT_LOAD_NO_HINTING | FT_LOAD_TARGET_NORMAL),
  myUChar       (0U),
  myToUseUnicodeSubsetFallback (Font_FontMgr::ToUseUnicodeSubsetFallback())
{
  if (myFTLib.IsNull())
  {
    myFTLib = new Font_FTLibrary();
  }
}

// =======================================================================
// function : Font_FTFont
// purpose  :
// =======================================================================
Font_FTFont::~Font_FTFont()
{
  Release();
}

// =======================================================================
// function : Release
// purpose  :
// =======================================================================
void Font_FTFont::Release()
{
  myGlyphImg.Clear();
  myFontPath.Clear();
  myUChar = 0;
  if (myFTFace != NULL)
  {
    FT_Done_Face (myFTFace);
    myFTFace = NULL;
  }
  myActiveFTFace = NULL;
  myBuffer.Nullify();
}

// =======================================================================
// function : Init
// purpose  :
// =======================================================================
bool Font_FTFont::Init (const Handle(NCollection_Buffer)& theData,
                        const TCollection_AsciiString& theFileName,
                        const Font_FTFontParams& theParams,
                        const Standard_Integer theFaceId)
{
  Release();
  myBuffer = theData;
  myFontPath = theFileName;
  myFontParams = theParams;
  if (!myFTLib->IsValid())
  {
    Message::DefaultMessenger()->Send ("FreeType library is unavailable", Message_Trace);
    Release();
    return false;
  }

  if (!theData.IsNull())
  {
    if (FT_New_Memory_Face (myFTLib->Instance(), theData->Data(), (FT_Long )theData->Size(), (FT_Long )theFaceId, &myFTFace) != 0)
    {
      Message::DefaultMessenger()->Send (TCollection_AsciiString("Font '") + myFontPath + "' failed to load from memory", Message_Trace);
      Release();
      return false;
    }
  }
#if !defined(__EMSCRIPTEN__)
  else
  {
    if (FT_New_Face (myFTLib->Instance(), myFontPath.ToCString(), (FT_Long )theFaceId, &myFTFace) != 0)
    {
      //Message::DefaultMessenger()->Send (TCollection_AsciiString("Font '") + myFontPath + "' failed to load from file", Message_Trace);
      Release();
      return false;
    }
  }
#else
  else {
    Release();
    return false;
  }
#endif

  if (FT_Select_Charmap (myFTFace, ft_encoding_unicode) != 0)
  {
    Message::DefaultMessenger()->Send (TCollection_AsciiString("Font '") + myFontPath + "' doesn't contains Unicode charmap", Message_Trace);
    Release();
    return false;
  }
  else if (FT_Set_Char_Size (myFTFace, 0L, toFTPoints (theParams.PointSize), theParams.Resolution, theParams.Resolution) != 0)
  {
    Message::DefaultMessenger()->Send (TCollection_AsciiString("Font '") + myFontPath + "' doesn't contains Unicode charmap of requested size", Message_Trace);
    Release();
    return false;
  }

  if (theParams.ToSynthesizeItalic)
  {
    const double THE_SHEAR_ANGLE = 10.0 * M_PI / 180.0;

    FT_Matrix aMat;
    aMat.xx = FT_Fixed (Cos (-THE_SHEAR_ANGLE) * (1 << 16));
    aMat.xy = 0;
    aMat.yx = 0;
    aMat.yy = aMat.xx;

    FT_Fixed aFactor = FT_Fixed (Tan (THE_SHEAR_ANGLE) * (1 << 16));
    aMat.xy += FT_MulFix (aFactor, aMat.xx);

    FT_Set_Transform (myFTFace, &aMat, 0);
  }
  myActiveFTFace = myFTFace;
  return true;
}

// =======================================================================
// function : FindAndCreate
// purpose  :
// =======================================================================
Handle(Font_FTFont) Font_FTFont::FindAndCreate (const TCollection_AsciiString& theFontName,
                                                const Font_FontAspect     theFontAspect,
                                                const Font_FTFontParams&  theParams,
                                                const Font_StrictLevel    theStrictLevel)
{
  Handle(Font_FontMgr) aFontMgr = Font_FontMgr::GetInstance();
  Font_FontAspect aFontAspect = theFontAspect;
  Font_FTFontParams aParams = theParams;
  if (Handle(Font_SystemFont) aRequestedFont = aFontMgr->FindFont (theFontName, theStrictLevel, aFontAspect))
  {
    if (aRequestedFont->IsSingleStrokeFont())
    {
      aParams.IsSingleStrokeFont = true;
    }

    Standard_Integer aFaceId = 0;
    const TCollection_AsciiString& aPath = aRequestedFont->FontPathAny (aFontAspect, aParams.ToSynthesizeItalic, aFaceId);
    Handle(Font_FTFont) aFont = new Font_FTFont();
    if (aFont->Init (aPath, aParams, aFaceId))
    {
      aFont->myFontAspect = aFontAspect;
      return aFont;
    }
  }
  else if (theStrictLevel == Font_StrictLevel_Any)
  {
    switch (theFontAspect)
    {
      case Font_FontAspect_UNDEFINED:
      case Font_FontAspect_Regular:
      case Font_FontAspect_Bold:
        aFontAspect = Font_FontAspect_Regular;
        break;
      case Font_FontAspect_Italic:
      case Font_FontAspect_BoldItalic:
        aFontAspect = Font_FontAspect_Italic;
        aParams.ToSynthesizeItalic = true;
        break;
    }
    Handle(NCollection_Buffer) aBuffer = new NCollection_Buffer (Handle(NCollection_BaseAllocator)(),
                                                                 Font_DejavuSans_Latin_woff_size,
                                                                 const_cast<Standard_Byte*>(Font_DejavuSans_Latin_woff));
    Handle(Font_FTFont) aFont = new Font_FTFont();
    if (aFont->Init (aBuffer, "Embed Fallback Font", aParams, 0))
    {
      aFont->myFontAspect = aFontAspect;
      return aFont;
    }
  }
  return Handle(Font_FTFont)();
}

// =======================================================================
// function : FindAndInit
// purpose  :
// =======================================================================
bool Font_FTFont::FindAndInit (const TCollection_AsciiString& theFontName,
                               Font_FontAspect theFontAspect,
                               const Font_FTFontParams& theParams,
                               Font_StrictLevel theStrictLevel)
{
  Font_FTFontParams aParams = theParams;
  myFontAspect = theFontAspect;
  Handle(Font_FontMgr) aFontMgr = Font_FontMgr::GetInstance();
  if (Handle(Font_SystemFont) aRequestedFont = aFontMgr->FindFont (theFontName.ToCString(), theStrictLevel, myFontAspect))
  {
    if (aRequestedFont->IsSingleStrokeFont())
    {
      aParams.IsSingleStrokeFont = true;
    }

    Standard_Integer aFaceId = 0;
    const TCollection_AsciiString& aPath = aRequestedFont->FontPathAny (myFontAspect, aParams.ToSynthesizeItalic, aFaceId);
    return Init (aPath, aParams, aFaceId);
  }
  else if (theStrictLevel == Font_StrictLevel_Any)
  {
    if (theFontAspect == Font_FontAspect_Italic
     || theFontAspect == Font_FontAspect_BoldItalic)
    {
      aParams.ToSynthesizeItalic = true;
    }
    Handle(NCollection_Buffer) aBuffer = new NCollection_Buffer (Handle(NCollection_BaseAllocator)(),
                                                                 Font_DejavuSans_Latin_woff_size,
                                                                 const_cast<Standard_Byte*>(Font_DejavuSans_Latin_woff));
    return Init (aBuffer, "Embed Fallback Font", aParams, 0);
  }
  Release();
  return false;
}

// =======================================================================
// function : findAndInitFallback
// purpose  :
// =======================================================================
bool Font_FTFont::findAndInitFallback (Font_UnicodeSubset theSubset)
{
  if (!myFallbackFaces[theSubset].IsNull())
  {
    return myFallbackFaces[theSubset]->IsValid();
  }

  myFallbackFaces[theSubset] = new Font_FTFont (myFTLib);
  myFallbackFaces[theSubset]->myToUseUnicodeSubsetFallback = false; // no recursion

  Handle(Font_FontMgr) aFontMgr = Font_FontMgr::GetInstance();
  if (Handle(Font_SystemFont) aRequestedFont = aFontMgr->FindFallbackFont (theSubset, myFontAspect))
  {
    Font_FTFontParams aParams = myFontParams;
    aParams.IsSingleStrokeFont = aRequestedFont->IsSingleStrokeFont();

    Standard_Integer aFaceId = 0;
    const TCollection_AsciiString& aPath = aRequestedFont->FontPathAny (myFontAspect, aParams.ToSynthesizeItalic, aFaceId);
    if (myFallbackFaces[theSubset]->Init (aPath, aParams, aFaceId))
    {
      Message::DefaultMessenger()->Send (TCollection_AsciiString ("Font_FTFont, using fallback font '") + aRequestedFont->FontName() + "'"
                                      + " for symbols unsupported by '" + myFTFace->family_name + "'", Message_Trace);
    }
  }
  return myFallbackFaces[theSubset]->IsValid();
}

// =======================================================================
// function : HasSymbol
// purpose  :
// =======================================================================
bool Font_FTFont::HasSymbol (Standard_Utf32Char theUChar) const
{
  return FT_Get_Char_Index (myFTFace, theUChar) != 0;
}

// =======================================================================
// function : loadGlyph
// purpose  :
// =======================================================================
bool Font_FTFont::loadGlyph (const Standard_Utf32Char theUChar)
{
  if (myUChar == theUChar)
  {
    return myUChar != 0;
  }

  myGlyphImg.Clear();
  myUChar = 0;
  myActiveFTFace = myFTFace;
  if (theUChar == 0)
  {
    return false;
  }

  if (myToUseUnicodeSubsetFallback
  && !HasSymbol (theUChar))
  {
    // try using fallback
    const Font_UnicodeSubset aSubset = CharSubset (theUChar);
    if (findAndInitFallback (aSubset)
     && myFallbackFaces[aSubset]->HasSymbol (theUChar))
    {
      myActiveFTFace = myFallbackFaces[aSubset]->myFTFace;
    }
  }

  if (FT_Load_Char (myActiveFTFace, theUChar, FT_Int32(myLoadFlags)) != 0
   || myActiveFTFace->glyph == NULL)
  {
    return false;
  }

  myUChar = theUChar;
  return true;
}

// =======================================================================
// function : RenderGlyph
// purpose  :
// =======================================================================
bool Font_FTFont::RenderGlyph (const Standard_Utf32Char theUChar)
{
  myGlyphImg.Clear();
  myUChar = 0;
  myActiveFTFace = myFTFace;

  if (theUChar != 0
  &&  myToUseUnicodeSubsetFallback
  && !HasSymbol (theUChar))
  {
    // try using fallback
    const Font_UnicodeSubset aSubset = CharSubset (theUChar);
    if (findAndInitFallback (aSubset)
     && myFallbackFaces[aSubset]->HasSymbol (theUChar))
    {
      myActiveFTFace = myFallbackFaces[aSubset]->myFTFace;
    }
  }

  if (theUChar == 0
   || FT_Load_Char (myActiveFTFace, theUChar, FT_Int32(myLoadFlags | FT_LOAD_RENDER)) != 0
   || myActiveFTFace->glyph == NULL
   || myActiveFTFace->glyph->format != FT_GLYPH_FORMAT_BITMAP)
  {
    return false;
  }

  FT_Bitmap aBitmap = myActiveFTFace->glyph->bitmap;
  if (aBitmap.buffer == NULL || aBitmap.width == 0 || aBitmap.rows == 0)
  {
    return false;
  }

  if (aBitmap.pixel_mode == FT_PIXEL_MODE_GRAY)
  {
    if (!myGlyphImg.InitWrapper (Image_Format_Alpha, aBitmap.buffer,
                                 aBitmap.width, aBitmap.rows, Abs (aBitmap.pitch)))
    {
      return false;
    }
    myGlyphImg.SetTopDown (aBitmap.pitch > 0);
  }
  else if (aBitmap.pixel_mode == FT_PIXEL_MODE_MONO)
  {
    if (!myGlyphImg.InitTrash (Image_Format_Gray, aBitmap.width, aBitmap.rows))
    {
      return false;
    }

    myGlyphImg.SetTopDown (aBitmap.pitch > 0);
    const int aNumOfBytesInRow = aBitmap.width / 8 + (aBitmap.width % 8 ? 1 : 0);
    for (int aRow = 0; aRow < (int )aBitmap.rows; ++aRow)
    {
      for (int aCol = 0; aCol < (int )aBitmap.width; ++aCol)
      {
        const int aBitOn = aBitmap.buffer[aNumOfBytesInRow * aRow + aCol / 8] & (0x80 >> (aCol % 8));
        *myGlyphImg.ChangeRawValue (aRow, aCol) = aBitOn ? 255 : 0;
      }
    }
  }
  else
  {
    return false;
  }

  myUChar = theUChar;
  return true;
}

// =======================================================================
// function : GlyphMaxSizeX
// purpose  :
// =======================================================================
unsigned int Font_FTFont::GlyphMaxSizeX (bool theToIncludeFallback) const
{
  if (!theToIncludeFallback)
  {
    float aWidth = (FT_IS_SCALABLE(myFTFace) != 0)
                 ? float(myFTFace->bbox.xMax - myFTFace->bbox.xMin) * (float(myFTFace->size->metrics.x_ppem) / float(myFTFace->units_per_EM))
                 : fromFTPoints<float> (myFTFace->size->metrics.max_advance);
    return (unsigned int)(aWidth + 0.5f);
  }

  unsigned int aWidth = GlyphMaxSizeX (false);
  if (theToIncludeFallback)
  {
    for (Standard_Integer aFontIter = 0; aFontIter < Font_UnicodeSubset_NB; ++aFontIter)
    {
      if (!myFallbackFaces[aFontIter].IsNull()
        && myFallbackFaces[aFontIter]->IsValid())
      {
        aWidth = std::max (aWidth, myFallbackFaces[aFontIter]->GlyphMaxSizeX (false));
      }
    }
  }
  return aWidth;
}

// =======================================================================
// function : GlyphMaxSizeY
// purpose  :
// =======================================================================
unsigned int Font_FTFont::GlyphMaxSizeY (bool theToIncludeFallback) const
{
  if (!theToIncludeFallback)
  {
    float aHeight = (FT_IS_SCALABLE(myFTFace) != 0)
                  ? float(myFTFace->bbox.yMax - myFTFace->bbox.yMin) * (float(myFTFace->size->metrics.y_ppem) / float(myFTFace->units_per_EM))
                  : fromFTPoints<float> (myFTFace->size->metrics.height);
    return (unsigned int)(aHeight + 0.5f);
  }

  unsigned int aHeight = GlyphMaxSizeY (false);
  if (theToIncludeFallback)
  {
    for (Standard_Integer aFontIter = 0; aFontIter < Font_UnicodeSubset_NB; ++aFontIter)
    {
      if (!myFallbackFaces[aFontIter].IsNull()
        && myFallbackFaces[aFontIter]->IsValid())
      {
        aHeight = std::max (aHeight, myFallbackFaces[aFontIter]->GlyphMaxSizeY (false));
      }
    }
  }
  return aHeight;
}

// =======================================================================
// function : Ascender
// purpose  :
// =======================================================================
float Font_FTFont::Ascender() const
{
  return float(myFTFace->ascender) * (float(myFTFace->size->metrics.y_ppem) / float(myFTFace->units_per_EM));
}

// =======================================================================
// function : Descender
// purpose  :
// =======================================================================
float Font_FTFont::Descender() const
{
  return float(myFTFace->descender) * (float(myFTFace->size->metrics.y_ppem) / float(myFTFace->units_per_EM));
}

// =======================================================================
// function : LineSpacing
// purpose  :
// =======================================================================
float Font_FTFont::LineSpacing() const
{
  return float(myFTFace->height) * (float(myFTFace->size->metrics.y_ppem) / float(myFTFace->units_per_EM));
}

// =======================================================================
// function : AdvanceX
// purpose  :
// =======================================================================
float Font_FTFont::AdvanceX (Standard_Utf32Char theUChar,
                             Standard_Utf32Char theUCharNext)
{
  loadGlyph (theUChar);
  return AdvanceX (theUCharNext);
}

// =======================================================================
// function : AdvanceY
// purpose  :
// =======================================================================
float Font_FTFont::AdvanceY (Standard_Utf32Char theUChar,
                             Standard_Utf32Char theUCharNext)
{
  loadGlyph (theUChar);
  return AdvanceY (theUCharNext);
}

// =======================================================================
// function : getKerning
// purpose  :
// =======================================================================
bool Font_FTFont::getKerning (FT_Vector& theKern,
                              Standard_Utf32Char theUCharCurr,
                              Standard_Utf32Char theUCharNext) const
{
  theKern.x = 0;
  theKern.y = 0;
  if (theUCharNext != 0 && FT_HAS_KERNING(myActiveFTFace) != 0)
  {
    const FT_UInt aCharCurr = FT_Get_Char_Index (myActiveFTFace, theUCharCurr);
    const FT_UInt aCharNext = FT_Get_Char_Index (myActiveFTFace, theUCharNext);
    if (aCharCurr == 0 || aCharNext == 0
     || FT_Get_Kerning (myActiveFTFace, aCharCurr, aCharNext, FT_KERNING_UNFITTED, &theKern) != 0)
    {
      theKern.x = 0;
      theKern.y = 0;
      return false;
    }
    return true;
  }
  return false;
}

// =======================================================================
// function : AdvanceX
// purpose  :
// =======================================================================
float Font_FTFont::AdvanceX (Standard_Utf32Char theUCharNext) const
{
  if (myUChar == 0)
  {
    return 0.0f;
  }

  FT_Vector aKern;
  getKerning (aKern, myUChar, theUCharNext);
  return myWidthScaling * fromFTPoints<float> (myActiveFTFace->glyph->advance.x + aKern.x);
}

// =======================================================================
// function : AdvanceY
// purpose  :
// =======================================================================
float Font_FTFont::AdvanceY (Standard_Utf32Char theUCharNext) const
{
  if (myUChar == 0)
  {
    return 0.0f;
  }

  FT_Vector aKern;
  getKerning (aKern, myUChar, theUCharNext);
  return fromFTPoints<float> (myActiveFTFace->glyph->advance.y + aKern.y);
}

// =======================================================================
// function : GlyphsNumber
// purpose  :
// =======================================================================
Standard_Integer Font_FTFont::GlyphsNumber (bool theToIncludeFallback) const
{
  Standard_Integer aNbGlyphs = myFTFace->num_glyphs;
  if (theToIncludeFallback)
  {
    for (Standard_Integer aFontIter = 0; aFontIter < Font_UnicodeSubset_NB; ++aFontIter)
    {
      if (!myFallbackFaces[aFontIter].IsNull()
        && myFallbackFaces[aFontIter]->IsValid())
      {
        aNbGlyphs += myFallbackFaces[aFontIter]->GlyphsNumber (false);
      }
    }
  }
  return aNbGlyphs;
}

// =======================================================================
// function : GlyphRect
// purpose  :
// =======================================================================
void Font_FTFont::GlyphRect (Font_Rect& theRect) const
{
  const FT_Bitmap& aBitmap = myActiveFTFace->glyph->bitmap;
  theRect.Left   = float(myActiveFTFace->glyph->bitmap_left);
  theRect.Top    = float(myActiveFTFace->glyph->bitmap_top);
  theRect.Right  = float(myActiveFTFace->glyph->bitmap_left + (int )aBitmap.width);
  theRect.Bottom = float(myActiveFTFace->glyph->bitmap_top  - (int )aBitmap.rows);
}

// =======================================================================
// function : BoundingBox
// purpose  :
// =======================================================================
Font_Rect Font_FTFont::BoundingBox (const NCollection_String&               theString,
                                    const Graphic3d_HorizontalTextAlignment theAlignX,
                                    const Graphic3d_VerticalTextAlignment   theAlignY)
{
  Font_TextFormatter aFormatter;
  aFormatter.SetupAlignment (theAlignX, theAlignY);
  aFormatter.Reset();

  aFormatter.Append (theString, *this);
  aFormatter.Format();

  Font_Rect aBndBox;
  aFormatter.BndBox (aBndBox);
  return aBndBox;
}

//! Initialize the font from the given file path.
//! @param theFontPath path to the font
//! @param theParams   initialization parameters
//! @return true on success
bool Font_FTFont::Init (const TCollection_AsciiString& theFontPath,
           const Font_FTFontParams& theParams)
{
#if !defined(__EMSCRIPTEN__)
  return Init (Handle(NCollection_Buffer)(), theFontPath, theParams);
#else
  Standard_Boolean aResult = Standard_False;
  Standard_Byte* file_data_buffer = nullptr;
  std::ifstream aDataStream(theFontPath.ToCString(), std::ios::in | std::ios::ate | std::ios::binary);
  if (aDataStream) {
    auto aFileSize = aDataStream.tellg();
    aDataStream.seekg(0);
    if(aDataStream) {
      file_data_buffer = new Standard_Byte[aFileSize+decltype(aFileSize)(1)];
      file_data_buffer[aFileSize] = 0;
      aDataStream.read((char*)file_data_buffer,aFileSize);
      if (aDataStream) {
        Handle(NCollection_Buffer) aBuffer = new NCollection_Buffer (Handle(NCollection_BaseAllocator)(),
                                                                     aFileSize,
                                                                     file_data_buffer);
        aResult = Init(aBuffer, theFontPath, theParams);
      }
    }
    aDataStream.close();
  }

  return aResult;
#endif
}
