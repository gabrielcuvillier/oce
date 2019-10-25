// Copied from OpenGl_View_Redraw, only enabling the GLES2 code path
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

#include <stdio.h>
#include <stdlib.h>

#include <OpenGl_GlCore11.hxx>

#include <Graphic3d_GraphicDriver.hxx>
#include <Graphic3d_StructureManager.hxx>
#include <Graphic3d_TextureParams.hxx>
#include <Graphic3d_Texture2Dmanual.hxx>
#include <Graphic3d_TransformUtils.hxx>
#include <Image_AlienPixMap.hxx>

#include <NCollection_Mat4.hxx>

#include <OpenGl_Context.hxx>
#include <OpenGl_FrameStats.hxx>
#include <OpenGl_Matrix.hxx>
#include <OpenGl_Workspace.hxx>
#include <OpenGl_View.hxx>
#include <OpenGl_GraduatedTrihedron.hxx>
#include <OpenGl_PrimitiveArray.hxx>
#include <OpenGl_ShaderManager.hxx>
#include <OpenGl_ShaderProgram.hxx>
#include <OpenGl_Structure.hxx>
#include <OpenGl_ArbFBO.hxx>

#if defined(GL_ES_VERSION_2_0)

namespace
{
  //! Format Frame Buffer format for logging messages.
  static TCollection_AsciiString printFboFormat (const Handle(OpenGl_FrameBuffer)& theFbo)
  {
    return TCollection_AsciiString() + theFbo->GetInitVPSizeX() + "x" + theFbo->GetInitVPSizeY() + "@" + theFbo->NbSamples();
  }

  //! Return TRUE if Frame Buffer initialized has failed with the same parameters.
  static bool checkWasFailedFbo (const Handle(OpenGl_FrameBuffer)& theFboToCheck,
                                 Standard_Integer theSizeX,
                                 Standard_Integer theSizeY,
                                 Standard_Integer theNbSamples)
  {
    return !theFboToCheck->IsValid()
        &&  theFboToCheck->GetInitVPSizeX() == theSizeX
        &&  theFboToCheck->GetInitVPSizeY() == theSizeY
        &&  theFboToCheck->NbSamples()      == theNbSamples;
  }
}

//=======================================================================
//function : drawBackground
//purpose  :
//=======================================================================
void OpenGl_View::drawBackground (const Handle(OpenGl_Workspace)& theWorkspace)
{
  const Handle(OpenGl_Context)& aCtx = theWorkspace->GetGlContext();
  const Standard_Boolean wasUsedZBuffer = theWorkspace->SetUseZBuffer (Standard_False);
  if (wasUsedZBuffer)
  {
    aCtx->core11fwd->glDisable (GL_DEPTH_TEST);
  }

  if (myBackgroundType == Graphic3d_TOB_CUBEMAP)
  {
    Graphic3d_Camera aCamera (theWorkspace->View()->Camera());
    aCamera.SetZRange (0.01, 1.0); // is needed to avoid perspective camera exception
    aCamera.SetProjectionType (Graphic3d_Camera::Projection_Perspective);

    aCtx->ProjectionState.Push();
    aCtx->ProjectionState.SetCurrent (aCamera.ProjectionMatrixF());

    myCubeMapParams->Aspect()->ShaderProgram()->PushVariableInt ("uZCoeff", myBackgroundCubeMap->ZIsInverted() ? -1 : 1);
    myCubeMapParams->Aspect()->ShaderProgram()->PushVariableInt ("uYCoeff", myBackgroundCubeMap->IsTopDown() ? 1 : -1);
    const OpenGl_Aspects* anOldAspectFace = theWorkspace->SetAspects (myCubeMapParams);

    myBackgrounds[Graphic3d_TOB_CUBEMAP]->Render (theWorkspace);

    aCtx->ProjectionState.Pop();
    aCtx->ApplyProjectionMatrix();
    theWorkspace->SetAspects (anOldAspectFace);
  }
  else if (myBackgroundType == Graphic3d_TOB_GRADIENT
        || myBackgroundType == Graphic3d_TOB_TEXTURE)
  {
    // Drawing background gradient if:
    // - gradient fill type is not Aspect_GFM_NONE and
    // - either background texture is no specified or it is drawn in Aspect_FM_CENTERED mode
    if (myBackgrounds[Graphic3d_TOB_GRADIENT]->IsDefined()
      && (!myTextureParams->Aspect()->ToMapTexture()
        || myBackgrounds[Graphic3d_TOB_TEXTURE]->TextureFillMethod() == Aspect_FM_CENTERED
        || myBackgrounds[Graphic3d_TOB_TEXTURE]->TextureFillMethod() == Aspect_FM_NONE))
    {
      myBackgrounds[Graphic3d_TOB_GRADIENT]->Render(theWorkspace);
    }

    // Drawing background image if it is defined
    // (texture is defined and fill type is not Aspect_FM_NONE)
    if (myBackgrounds[Graphic3d_TOB_TEXTURE]->IsDefined()
      && myTextureParams->Aspect()->ToMapTexture())
    {
      aCtx->core11fwd->glDisable (GL_BLEND);

      const OpenGl_Aspects* anOldAspectFace = theWorkspace->SetAspects (myTextureParams);
      myBackgrounds[Graphic3d_TOB_TEXTURE]->Render (theWorkspace);
      theWorkspace->SetAspects (anOldAspectFace);
    }
  }

  if (wasUsedZBuffer)
  {
    theWorkspace->SetUseZBuffer (Standard_True);
    aCtx->core11fwd->glEnable (GL_DEPTH_TEST);
  }
}

//=======================================================================
//function : Redraw
//purpose  :
//=======================================================================
void OpenGl_View::Redraw()
{
  const Standard_Boolean wasDisabledMSAA = myToDisableMSAA;
  const Standard_Boolean hadFboBlit      = myHasFboBlit;

  if (!myWorkspace->Activate())
  {
    return;
  }

  myWindow->SetSwapInterval();

  ++myFrameCounter;
  Graphic3d_Camera::Projection aProjectType = myCamera->ProjectionType();
  Handle(OpenGl_Context)       aCtx         = myWorkspace->GetGlContext();
  aCtx->FrameStats()->FrameStart (myWorkspace->View(), false);
  aCtx->SetLineFeather (myRenderParams.LineFeather);

  // release pending GL resources
  aCtx->ReleaseDelayed();

  // fetch OpenGl context state
  aCtx->FetchState();

  OpenGl_FrameBuffer* aFrameBuffer = myFBO.get();

#if !defined(HAVE_WEBGL)
  bool toSwap = aCtx->IsRender()
            && !aCtx->caps->buffersNoSwap
            &&  aFrameBuffer == NULL;
#else
  const bool toSwap = false;
#endif

  const Standard_Integer aSizeX = aFrameBuffer != NULL ? aFrameBuffer->GetVPSizeX() : myWindow->Width();
  const Standard_Integer aSizeY = aFrameBuffer != NULL ? aFrameBuffer->GetVPSizeY() : myWindow->Height();
  const Standard_Integer aRendSizeX = Standard_Integer(myRenderParams.RenderResolutionScale * aSizeX + 0.5f);
  const Standard_Integer aRendSizeY = Standard_Integer(myRenderParams.RenderResolutionScale * aSizeY + 0.5f);
  if (aSizeX < 1
   || aSizeY < 1
   || aRendSizeX < 1
   || aRendSizeY < 1)
  {
    myBackBufferRestored = Standard_False;
    myIsImmediateDrawn   = Standard_False;
    return;
  }

  // determine multisampling parameters
#if !defined(HAVE_WEBGL)
  Standard_Integer aNbSamples = !myToDisableMSAA && aSizeX == aRendSizeX
                              ? Max (Min (myRenderParams.NbMsaaSamples, aCtx->MaxMsaaSamples()), 0)
                              : 0;
  if (aNbSamples != 0)
  {
    aNbSamples = OpenGl_Context::GetPowerOfTwo (aNbSamples, aCtx->MaxMsaaSamples());
  }
#else
  const Standard_Integer aNbSamples = 0;
#endif

  if ( aFrameBuffer == NULL
   && !aCtx->DefaultFrameBuffer().IsNull()
   &&  aCtx->DefaultFrameBuffer()->IsValid())
  {
    aFrameBuffer = aCtx->DefaultFrameBuffer().operator->();
  }

  if (myHasFboBlit && !aCtx->caps->fboDisable
   && (myTransientDrawToFront
    || aProjectType == Graphic3d_Camera::Projection_Stereo
    || aNbSamples != 0
    || aSizeX != aRendSizeX))
  {
    if (myMainSceneFbos[0]->GetVPSizeX() != aRendSizeX
     || myMainSceneFbos[0]->GetVPSizeY() != aRendSizeY
     || myMainSceneFbos[0]->NbSamples()  != aNbSamples)
    {
      // prepare FBOs containing main scene
      // for further blitting and rendering immediate presentations on top
      if (aCtx->core20fwd != NULL)
      {
        const bool wasFailedMain0 = checkWasFailedFbo (myMainSceneFbos[0], aRendSizeX, aRendSizeY, aNbSamples);
        if (!myMainSceneFbos[0]->Init (aCtx, aRendSizeX, aRendSizeY, myFboColorFormat, myFboDepthFormat, aNbSamples)
         && !wasFailedMain0)
        {
          TCollection_ExtendedString aMsg = TCollection_ExtendedString() + "Error! Main FBO "
                                          + printFboFormat (myMainSceneFbos[0]) + " initialization has failed";
          aCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH, aMsg);
        }
      }
    }
  }
  else
  {
    myMainSceneFbos     [0]->Release (aCtx.operator->());
    myMainSceneFbos     [1]->Release (aCtx.operator->());
    myMainSceneFbos     [0]->ChangeViewport (0, 0);
    myMainSceneFbos     [1]->ChangeViewport (0, 0);
  }

  {
    OpenGl_FrameBuffer* aMainFbo    = myMainSceneFbos[0]->IsValid() ? myMainSceneFbos[0].operator->() : aFrameBuffer;
    OpenGl_FrameBuffer* anImmFbo    = aFrameBuffer;
    if (!myTransientDrawToFront)
    {
      anImmFbo    = aMainFbo;
    }

    aCtx->SetResolution (myRenderParams.Resolution, myRenderParams.ResolutionRatio(),
                         aMainFbo != aFrameBuffer ? myRenderParams.RenderResolutionScale : 1.0f);

    redraw (aProjectType, aMainFbo, NULL);
    myBackBufferRestored = Standard_True;
    myIsImmediateDrawn   = Standard_False;
    aCtx->SetResolution (myRenderParams.Resolution, myRenderParams.ResolutionRatio(),
                         anImmFbo != aFrameBuffer ? myRenderParams.RenderResolutionScale : 1.0f);
    if (!redrawImmediate (aProjectType, aMainFbo, anImmFbo, NULL))
    {
#if !defined(HAVE_WEBGL)
      toSwap = false;
#endif
    }

    if (anImmFbo != NULL
     && anImmFbo != aFrameBuffer)
    {
      blitBuffers (anImmFbo, aFrameBuffer, myToFlipOutput);
    }
  }

  // bind default FBO
  bindDefaultFbo();

  if (wasDisabledMSAA != myToDisableMSAA
   || hadFboBlit      != myHasFboBlit)
  {
    // retry on error
    Redraw();
  }

  // reset state for safety
  aCtx->BindProgram (Handle(OpenGl_ShaderProgram)());

  // Swap the buffers
  if (toSwap)
  {
    aCtx->SwapBuffers();
    if (!myMainSceneFbos[0]->IsValid())
    {
      myBackBufferRestored = Standard_False;
    }
  }
  else
  {
#if !defined(HAVE_WEBGL)
    aCtx->core11fwd->glFlush();
#endif
  }

  // reset render mode state
  aCtx->FetchState();
  aCtx->FrameStats()->FrameEnd (myWorkspace->View(), false);

  myWasRedrawnGL = Standard_True;
}

// =======================================================================
// function : RedrawImmediate
// purpose  :
// =======================================================================
void OpenGl_View::RedrawImmediate()
{
  if (!myWorkspace->Activate())
    return;

  Handle(OpenGl_Context) aCtx = myWorkspace->GetGlContext();
  if (!myTransientDrawToFront
   || !myBackBufferRestored
   || (aCtx->caps->buffersNoSwap && !myMainSceneFbos[0]->IsValid()))
  {
    Redraw();
    return;
  }

  Graphic3d_Camera::Projection aProjectType = myCamera->ProjectionType();
  OpenGl_FrameBuffer*          aFrameBuffer = myFBO.get();
  aCtx->FrameStats()->FrameStart (myWorkspace->View(), true);

  if ( aFrameBuffer == NULL
   && !aCtx->DefaultFrameBuffer().IsNull()
   &&  aCtx->DefaultFrameBuffer()->IsValid())
  {
    aFrameBuffer = aCtx->DefaultFrameBuffer().operator->();
  }

#if !defined(HAVE_WEBGL)
  bool toSwap = false;
#else
  const bool toSwap = false;
#endif
  {
    OpenGl_FrameBuffer* aMainFbo = myMainSceneFbos[0]->IsValid() ? myMainSceneFbos[0].operator->() : NULL;
    OpenGl_FrameBuffer* anImmFbo = aFrameBuffer;
    aCtx->SetResolution (myRenderParams.Resolution, myRenderParams.ResolutionRatio(),
                         anImmFbo != aFrameBuffer ? myRenderParams.RenderResolutionScale : 1.0f);
#if !defined(HAVE_WEBGL)
    toSwap = redrawImmediate (aProjectType,
                              aMainFbo,
                              anImmFbo,
                              NULL) || toSwap;
#else
    redrawImmediate ( aProjectType,
                      aMainFbo,
                      anImmFbo,
                      NULL);
#endif

    if (anImmFbo != NULL
     && anImmFbo != aFrameBuffer)
    {
      blitBuffers (anImmFbo, aFrameBuffer, myToFlipOutput);
    }
  }
  // bind default FBO
  bindDefaultFbo();

  // reset state for safety
  aCtx->BindProgram (Handle(OpenGl_ShaderProgram)());

  if (toSwap && !aCtx->caps->buffersNoSwap)
  {
    aCtx->SwapBuffers();
  }
  else
  {
#if !defined(HAVE_WEBGL)
    aCtx->core11fwd->glFlush();
#endif
  }
  aCtx->FrameStats()->FrameEnd (myWorkspace->View(), true);

  myWasRedrawnGL = Standard_True;
}

// =======================================================================
// function : redraw
// purpose  :
// =======================================================================
void OpenGl_View::redraw (const Graphic3d_Camera::Projection theProjection,
                          OpenGl_FrameBuffer*                theReadDrawFbo,
                          OpenGl_FrameBuffer*                /*theOitAccumFbo*/)
{
  Handle(OpenGl_Context) aCtx = myWorkspace->GetGlContext();
  if (theReadDrawFbo != NULL)
  {
    theReadDrawFbo->BindBuffer    (aCtx);
    theReadDrawFbo->SetupViewport (aCtx);
  }
  else
  {
    const Standard_Integer aViewport[4] = { 0, 0, myWindow->Width(), myWindow->Height() };
    aCtx->ResizeViewport (aViewport);
  }

  // request reset of material
  aCtx->ShaderManager()->UpdateMaterialState();

  myWorkspace->UseZBuffer()    = Standard_True;
  myWorkspace->UseDepthWrite() = Standard_True;
  GLbitfield toClear = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
  glDepthFunc (GL_LEQUAL);
  glDepthMask (GL_TRUE);
  glEnable (GL_DEPTH_TEST);

  glClearDepthf (1.0f);

  const OpenGl_Vec4& aBgColor = myBgColor;
  glClearColor (aBgColor.r(), aBgColor.g(), aBgColor.b(), 0.0f);

  glClear (toClear);

  render (theProjection, theReadDrawFbo, NULL, Standard_False);
}

// =======================================================================
// function : redrawImmediate
// purpose  :
// =======================================================================
bool OpenGl_View::redrawImmediate (const Graphic3d_Camera::Projection theProjection,
                                   OpenGl_FrameBuffer*                theReadFbo,
                                   OpenGl_FrameBuffer*                theDrawFbo,
                                   OpenGl_FrameBuffer*                /*theOitAccumFbo*/,
                                   const Standard_Boolean             theIsPartialUpdate)
{
  const Handle(OpenGl_Context)& aCtx = myWorkspace->GetGlContext();
  GLboolean toCopyBackToFront = GL_FALSE;
  if (theDrawFbo == theReadFbo
   && theDrawFbo != NULL)
  {
    myBackBufferRestored = Standard_False;
  }
  else if (theReadFbo != NULL
        && theReadFbo->IsValid()
        && aCtx->IsRender())
  {
    if (!blitBuffers (theReadFbo, theDrawFbo))
    {
      return true;
    }
  }
  else if (theDrawFbo == NULL)
  {
#if !defined(HAVE_WEBGL)
    aCtx->core11fwd->glGetBooleanv (GL_DOUBLEBUFFER, &toCopyBackToFront);
    if (toCopyBackToFront
     && myTransientDrawToFront)
    {
      if (!HasImmediateStructures()
        && !theIsPartialUpdate)
      {
        // prefer Swap Buffers within Redraw in compatibility mode (without FBO)
        return true;
      }
      if (!copyBackToFront())
      {
        toCopyBackToFront    = GL_FALSE;
        myBackBufferRestored = Standard_False;
      }
    }
    else
    {
      toCopyBackToFront    = GL_FALSE;
      myBackBufferRestored = Standard_False;
    }
#endif
  }
  else
  {
    myBackBufferRestored = Standard_False;
  }
  myIsImmediateDrawn = Standard_True;

  myWorkspace->UseZBuffer()    = Standard_True;
  myWorkspace->UseDepthWrite() = Standard_True;
  glDepthFunc (GL_LEQUAL);
  glDepthMask (GL_TRUE);
  glEnable (GL_DEPTH_TEST);
  glClearDepthf (1.0f);

  render (theProjection, theDrawFbo, NULL, Standard_True);

  return !toCopyBackToFront;
}

//=======================================================================
//function : Render
//purpose  :
//=======================================================================
void OpenGl_View::render (Graphic3d_Camera::Projection theProjection,
                          OpenGl_FrameBuffer*          theOutputFBO,
                          OpenGl_FrameBuffer*          /*theOitAccumFbo*/,
                          const Standard_Boolean       theToDrawImmediate)
{
  // ==================================
  //      Step 1: Prepare for render
  // ==================================

  const Handle(OpenGl_Context)& aContext = myWorkspace->GetGlContext();
  aContext->SetAllowSampleAlphaToCoverage (myRenderParams.ToEnableAlphaToCoverage
                                        && theOutputFBO != NULL
                                        && theOutputFBO->NbSamples() != 0);

  // update states of OpenGl_BVHTreeSelector (frustum culling algorithm);
  // note that we pass here window dimensions ignoring Graphic3d_RenderingParams::RenderResolutionScale
  myBVHSelector.SetViewVolume (myCamera);
  myBVHSelector.SetViewportSize (myWindow->Width(), myWindow->Height(), myRenderParams.ResolutionRatio());
  myBVHSelector.CacheClipPtsProjections();

  const Handle(OpenGl_ShaderManager)& aManager = aContext->ShaderManager();
  const Handle(Graphic3d_LightSet)&   aLights  = myShadingModel == Graphic3d_TOSM_UNLIT ? myNoShadingLight : myLights;
  Standard_Size aLightsRevision = 0;
  if (!aLights.IsNull())
  {
    aLightsRevision = aLights->UpdateRevision();
  }
  if (StateInfo (myCurrLightSourceState, aManager->LightSourceState().Index()) != myLastLightSourceState
   || aLightsRevision != myLightsRevision)
  {
    myLightsRevision = aLightsRevision;
    aManager->UpdateLightSourceStateTo (aLights);
    myLastLightSourceState = StateInfo (myCurrLightSourceState, aManager->LightSourceState().Index());
  }

  // Update matrices if camera has changed.
  Graphic3d_WorldViewProjState aWVPState = myCamera->WorldViewProjState();
  if (myWorldViewProjState != aWVPState)
  {
    myWorldViewProjState = aWVPState;
  }

  myLocalOrigin.SetCoord (0.0, 0.0, 0.0);
  aContext->ProjectionState.SetCurrent (myCamera->ProjectionMatrixF());
  aContext->WorldViewState .SetCurrent (myCamera->OrientationMatrixF());
  aContext->ApplyProjectionMatrix();
  aContext->ApplyWorldViewMatrix();
  if (aManager->ModelWorldState().Index() == 0)
  {
    aContext->ShaderManager()->UpdateModelWorldStateTo (OpenGl_Mat4());
  }

  // ====================================
  //      Step 2: Redraw background
  // ====================================

  // Render background
  if (!theToDrawImmediate)
  {
    drawBackground (myWorkspace);
  }

  // =================================
  //      Step 3: Redraw main plane
  // =================================

  // Setup face culling
  GLboolean isCullFace = GL_FALSE;
  if (myBackfacing != Graphic3d_TOBM_AUTOMATIC)
  {
    isCullFace = glIsEnabled (GL_CULL_FACE);
    if (myBackfacing == Graphic3d_TOBM_DISABLE)
    {
      glEnable (GL_CULL_FACE);
      glCullFace (GL_BACK);
    }
    else
      glDisable (GL_CULL_FACE);
  }

  aManager->SetShadingModel (myShadingModel);

  // Redraw 3d scene
  if (theProjection == Graphic3d_Camera::Projection_MonoLeftEye)
  {
    aContext->ProjectionState.SetCurrent (myCamera->ProjectionStereoLeftF());
    aContext->ApplyProjectionMatrix();
  }
  else if (theProjection == Graphic3d_Camera::Projection_MonoRightEye)
  {
    aContext->ProjectionState.SetCurrent (myCamera->ProjectionStereoRightF());
    aContext->ApplyProjectionMatrix();
  }

  myWorkspace->SetEnvironmentTexture (myTextureEnv);

  renderScene (theProjection, theOutputFBO, NULL, theToDrawImmediate);

  myWorkspace->SetEnvironmentTexture (Handle(OpenGl_TextureSet)());

  // ===============================
  //      Step 4: Trihedron
  // ===============================

  // Resetting GL parameters according to the default aspects
  // in order to synchronize GL state with the graphic driver state
  // before drawing auxiliary stuff (trihedrons, overlayer)
  myWorkspace->ResetAppliedAspect();

  // Render trihedron
  if (!theToDrawImmediate)
  {
    renderTrihedron (myWorkspace);

    // Restore face culling
    if (myBackfacing != Graphic3d_TOBM_AUTOMATIC)
    {
      if (isCullFace)
      {
        glEnable (GL_CULL_FACE);
        glCullFace (GL_BACK);
      }
      else
        glDisable (GL_CULL_FACE);
    }
  }
  else
  {
    renderFrameStats();
  }

  myWorkspace->ResetAppliedAspect();
  aContext->SetAllowSampleAlphaToCoverage (false);
  aContext->SetSampleAlphaToCoverage (false);

  // reset FFP state for safety
  aContext->BindProgram (Handle(OpenGl_ShaderProgram)());

  // ==============================================================
  //      Step 6: Keep shader manager informed about last View
  // ==============================================================

  if (!aManager.IsNull())
  {
    aManager->SetLastView (this);
  }
}

// =======================================================================
// function : InvalidateBVHData
// purpose  :
// =======================================================================
void OpenGl_View::InvalidateBVHData (const Graphic3d_ZLayerId theLayerId)
{
  myZLayers.InvalidateBVHData (theLayerId);
}

//=======================================================================
//function : renderStructs
//purpose  :
//=======================================================================
void OpenGl_View::renderStructs (Graphic3d_Camera::Projection /*theProjection*/,
                                 OpenGl_FrameBuffer*          theReadDrawFbo,
                                 OpenGl_FrameBuffer*          /*theOitAccumFbo*/,
                                 const Standard_Boolean       theToDrawImmediate)
{
  myZLayers.UpdateCulling (myWorkspace, theToDrawImmediate);
  if ( myZLayers.NbStructures() <= 0 )
    return;

  Handle(OpenGl_Context) aCtx = myWorkspace->GetGlContext();

  // Redraw 3D scene using OpenGL in standard
  // mode or in case of ray-tracing failure
  myZLayers.Render (myWorkspace, theToDrawImmediate, OpenGl_LF_All, theReadDrawFbo, NULL);

  // Set flag that scene was redrawn by standard pipeline
  myWasRedrawnGL = Standard_True;
}

//=======================================================================
//function : renderTrihedron
//purpose  :
//=======================================================================
void OpenGl_View::renderTrihedron (const Handle(OpenGl_Workspace) &theWorkspace)
{
  if (myToShowGradTrihedron)
  {
    myGraduatedTrihedron.Render (theWorkspace);
  }
}

//=======================================================================
//function : renderFrameStats
//purpose  :
//=======================================================================
void OpenGl_View::renderFrameStats()
{
  if (myRenderParams.ToShowStats
   && myRenderParams.CollectedStats != Graphic3d_RenderingParams::PerfCounters_NONE)
  {
    myFrameStatsPrs.Update (myWorkspace);
    myFrameStatsPrs.Render (myWorkspace);
  }
}

// =======================================================================
// function : Invalidate
// purpose  :
// =======================================================================
void OpenGl_View::Invalidate()
{
  myBackBufferRestored = Standard_False;
}

//=======================================================================
//function : renderScene
//purpose  :
//=======================================================================
void OpenGl_View::renderScene (Graphic3d_Camera::Projection theProjection,
                               OpenGl_FrameBuffer*          theReadDrawFbo,
                               OpenGl_FrameBuffer*          /*theOitAccumFbo*/,
                               const Standard_Boolean       theToDrawImmediate)
{
  const Handle(OpenGl_Context)& aContext = myWorkspace->GetGlContext();

  // Specify clipping planes in view transformation space
  aContext->ChangeClipping().Reset (myClipPlanes);
  if (!myClipPlanes.IsNull()
   && !myClipPlanes->IsEmpty())
  {
    aContext->ShaderManager()->UpdateClippingState();
  }

  renderStructs (theProjection, theReadDrawFbo, NULL, theToDrawImmediate);
  aContext->BindTextures (Handle(OpenGl_TextureSet)());

  // Apply restored view matrix.
  aContext->ApplyWorldViewMatrix();

  aContext->ChangeClipping().Reset (Handle(Graphic3d_SequenceOfHClipPlane)());
  if (!myClipPlanes.IsNull()
   && !myClipPlanes->IsEmpty())
  {
    aContext->ShaderManager()->RevertClippingState();
  }
}

// =======================================================================
// function : bindDefaultFbo
// purpose  :
// =======================================================================
void OpenGl_View::bindDefaultFbo (OpenGl_FrameBuffer* theCustomFbo)
{
  Handle(OpenGl_Context) aCtx = myWorkspace->GetGlContext();
  OpenGl_FrameBuffer* anFbo = (theCustomFbo != NULL && theCustomFbo->IsValid())
                            ?  theCustomFbo
                            : (!aCtx->DefaultFrameBuffer().IsNull()
                             && aCtx->DefaultFrameBuffer()->IsValid()
                              ? aCtx->DefaultFrameBuffer().operator->()
                              : NULL);
  if (anFbo != NULL)
  {
    anFbo->BindBuffer (aCtx);
    anFbo->SetupViewport (aCtx);
  }
  else
  {
    if (aCtx->arbFBO != NULL)
    {
      aCtx->arbFBO->glBindFramebuffer (GL_FRAMEBUFFER, OpenGl_FrameBuffer::NO_FRAMEBUFFER);
    }
    const Standard_Integer aViewport[4] = { 0, 0, myWindow->Width(), myWindow->Height() };
    aCtx->ResizeViewport (aViewport);
  }
}

// =======================================================================
// function : initBlitQuad
// purpose  :
// =======================================================================
OpenGl_VertexBuffer* OpenGl_View::initBlitQuad (const Standard_Boolean theToFlip)
{
  OpenGl_VertexBuffer* aVerts = NULL;
  if (!theToFlip)
  {
    aVerts = &myFullScreenQuad;
    if (!aVerts->IsValid())
    {
      OpenGl_Vec4 aQuad[4] =
      {
        OpenGl_Vec4( 1.0f, -1.0f, 1.0f, 0.0f),
        OpenGl_Vec4( 1.0f,  1.0f, 1.0f, 1.0f),
        OpenGl_Vec4(-1.0f, -1.0f, 0.0f, 0.0f),
        OpenGl_Vec4(-1.0f,  1.0f, 0.0f, 1.0f)
      };
      aVerts->Init (myWorkspace->GetGlContext(), 4, 4, aQuad[0].GetData());
    }
  }
  else
  {
    aVerts = &myFullScreenQuadFlip;
    if (!aVerts->IsValid())
    {
      OpenGl_Vec4 aQuad[4] =
      {
        OpenGl_Vec4( 1.0f, -1.0f, 1.0f, 1.0f),
        OpenGl_Vec4( 1.0f,  1.0f, 1.0f, 0.0f),
        OpenGl_Vec4(-1.0f, -1.0f, 0.0f, 1.0f),
        OpenGl_Vec4(-1.0f,  1.0f, 0.0f, 0.0f)
      };
      aVerts->Init (myWorkspace->GetGlContext(), 4, 4, aQuad[0].GetData());
    }
  }
  return aVerts;
}

// =======================================================================
// function : blitBuffers
// purpose  :
// =======================================================================
bool OpenGl_View::blitBuffers (OpenGl_FrameBuffer*    theReadFbo,
                               OpenGl_FrameBuffer*    theDrawFbo,
                               const Standard_Boolean theToFlip)
{
  Handle(OpenGl_Context) aCtx = myWorkspace->GetGlContext();
  const Standard_Integer aReadSizeX = theReadFbo != NULL ? theReadFbo->GetVPSizeX() : myWindow->Width();
  const Standard_Integer aReadSizeY = theReadFbo != NULL ? theReadFbo->GetVPSizeY() : myWindow->Height();
  const Standard_Integer aDrawSizeX = theDrawFbo != NULL ? theDrawFbo->GetVPSizeX() : myWindow->Width();
  const Standard_Integer aDrawSizeY = theDrawFbo != NULL ? theDrawFbo->GetVPSizeY() : myWindow->Height();
  if (theReadFbo == NULL || aCtx->IsFeedback())
  {
    return false;
  }
  else if (theReadFbo == theDrawFbo)
  {
    return true;
  }

  // clear destination before blitting
  if (theDrawFbo != NULL
  &&  theDrawFbo->IsValid())
  {
    theDrawFbo->BindBuffer (aCtx);
  }
  else
  {
    aCtx->arbFBO->glBindFramebuffer (GL_FRAMEBUFFER, OpenGl_FrameBuffer::NO_FRAMEBUFFER);
  }
  const Standard_Integer aViewport[4] = { 0, 0, aDrawSizeX, aDrawSizeY };
  aCtx->ResizeViewport (aViewport);

  aCtx->core20fwd->glClearDepthf (1.0f);
  aCtx->core20fwd->glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

#if !defined(HAVE_WEBGL)
  if (aCtx->arbFBOBlit != NULL
   && theReadFbo->NbSamples() != 0)
  {
    GLbitfield aCopyMask = 0;
    theReadFbo->BindReadBuffer (aCtx);
    if (theDrawFbo != NULL
     && theDrawFbo->IsValid())
    {
      theDrawFbo->BindDrawBuffer (aCtx);
      if (theDrawFbo->HasColor()
       && theReadFbo->HasColor())
      {
        aCopyMask |= GL_COLOR_BUFFER_BIT;
      }
      if (theDrawFbo->HasDepth()
       && theReadFbo->HasDepth())
      {
        aCopyMask |= GL_DEPTH_BUFFER_BIT;
      }
    }
    else
    {
      if (theReadFbo->HasColor())
      {
        aCopyMask |= GL_COLOR_BUFFER_BIT;
      }
      if (theReadFbo->HasDepth())
      {
        aCopyMask |= GL_DEPTH_BUFFER_BIT;
      }
      aCtx->arbFBO->glBindFramebuffer (GL_DRAW_FRAMEBUFFER, OpenGl_FrameBuffer::NO_FRAMEBUFFER);
    }

    // we don't copy stencil buffer here... does it matter for performance?
    aCtx->arbFBOBlit->glBlitFramebuffer (0, 0, aReadSizeX, aReadSizeY,
                                         0, 0, aDrawSizeX, aDrawSizeY,
                                         aCopyMask, GL_NEAREST);
    const int anErr = ::glGetError();
    if (anErr != GL_NO_ERROR)
    {
      // glBlitFramebuffer() might fail in several cases:
      // - Both FBOs have MSAA and they are samples number does not match.
      //   OCCT checks that this does not happen,
      //   however some graphics drivers provide an option for overriding MSAA.
      //   In this case window MSAA might be non-zero (and application can not check it)
      //   and might not match MSAA of our offscreen FBOs.
      // - Pixel formats of FBOs do not match.
      //   This also might happen with window has pixel format,
      //   e.g. Mesa fails blitting RGBA8 -> RGB8 while other drivers support this conversion.
      TCollection_ExtendedString aMsg = TCollection_ExtendedString() + "FBO blitting has failed [Error #" + anErr + "]\n"
                                      + "  Please check your graphics driver settings or try updating driver.";
      if (theReadFbo->NbSamples() != 0)
      {
        myToDisableMSAA = true;
        aMsg += "\n  MSAA settings should not be overridden by driver!";
      }
      aCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION,
                         GL_DEBUG_TYPE_ERROR,
                         0,
                         GL_DEBUG_SEVERITY_HIGH,
                         aMsg);
    }

    if (theDrawFbo != NULL
     && theDrawFbo->IsValid())
    {
      theDrawFbo->BindBuffer (aCtx);
    }
    else
    {
      aCtx->arbFBO->glBindFramebuffer (GL_FRAMEBUFFER, OpenGl_FrameBuffer::NO_FRAMEBUFFER);
    }
  }
  else
#endif
  {
    aCtx->core20fwd->glDepthFunc (GL_ALWAYS);
    aCtx->core20fwd->glDepthMask (GL_TRUE);
    aCtx->core20fwd->glEnable (GL_DEPTH_TEST);
    if (!aCtx->IsGlGreaterEqual (3, 0)
     && !aCtx->extFragDepth)
    {
      aCtx->core20fwd->glDisable (GL_DEPTH_TEST);
    }

    aCtx->BindTextures (Handle(OpenGl_TextureSet)());

    const Graphic3d_TypeOfTextureFilter aFilter = (aDrawSizeX == aReadSizeX && aDrawSizeY == aReadSizeY) ? Graphic3d_TOTF_NEAREST : Graphic3d_TOTF_BILINEAR;
    const GLint aFilterGl = aFilter == Graphic3d_TOTF_NEAREST ? GL_NEAREST : GL_LINEAR;

    OpenGl_VertexBuffer* aVerts = initBlitQuad (theToFlip);
    const Handle(OpenGl_ShaderManager)& aManager = aCtx->ShaderManager();
    if (aVerts->IsValid()
     && aManager->BindFboBlitProgram())
    {
      aCtx->SetSampleAlphaToCoverage (false);
      theReadFbo->ColorTexture()->Bind (aCtx, Graphic3d_TextureUnit_0);
      if (theReadFbo->ColorTexture()->Sampler()->Parameters()->Filter() != aFilter)
      {
        theReadFbo->ColorTexture()->Sampler()->Parameters()->SetFilter (aFilter);
        aCtx->core20fwd->glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, aFilterGl);
        aCtx->core20fwd->glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, aFilterGl);
      }

      theReadFbo->DepthStencilTexture()->Bind (aCtx, Graphic3d_TextureUnit_1);
      if (theReadFbo->DepthStencilTexture()->Sampler()->Parameters()->Filter() != aFilter)
      {
        theReadFbo->DepthStencilTexture()->Sampler()->Parameters()->SetFilter (aFilter);
        aCtx->core20fwd->glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, aFilterGl);
        aCtx->core20fwd->glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, aFilterGl);
      }

      aVerts->BindVertexAttrib (aCtx, Graphic3d_TOA_POS);

      aCtx->core20fwd->glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);

      aVerts->UnbindVertexAttrib (aCtx, Graphic3d_TOA_POS);
      theReadFbo->DepthStencilTexture()->Unbind (aCtx, Graphic3d_TextureUnit_1);
      theReadFbo->ColorTexture()       ->Unbind (aCtx, Graphic3d_TextureUnit_0);
      aCtx->BindProgram (NULL);
    }
    else
    {
      TCollection_ExtendedString aMsg = TCollection_ExtendedString()
        + "Error! FBO blitting has failed";
      aCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION,
                         GL_DEBUG_TYPE_ERROR,
                         0,
                         GL_DEBUG_SEVERITY_HIGH,
                         aMsg);
      myHasFboBlit = Standard_False;
      theReadFbo->Release (aCtx.operator->());
      return true;
    }
  }
  return true;
}

// =======================================================================
// function : copyBackToFront
// purpose  :
// =======================================================================
bool OpenGl_View::copyBackToFront()
{
  myIsImmediateDrawn = Standard_False;
  return false;
}

#endif