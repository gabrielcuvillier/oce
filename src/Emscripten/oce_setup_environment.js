// Copyright (c) 2019 Gabriel Cuvillier - Continuation Labs

if (Module['preRun'] instanceof Array) {
  Module['preRun'].push(oce_setup_environment);
} else {
  Module['preRun'] = [oce_setup_environment];
}

function oce_setup_environment() {
  FS.createPath('/', 'oce', true, true);
  ENV.CASROOT = "/oce";
  ENV.CSF_LANGUAGE = "us";
  // Units
  ENV.CSF_UnitsLexicon = "/oce/src/UnitsAPI/Lexi_Expr.dat";
  ENV.CSF_UnitsDefinition	= "/oce/src/UnitsAPI/Units.dat";
  ENV.CSF_CurrentUnitsDefaults =    "/oce/src/StdResource";
  ENV.CSF_MDTVCurrentUnitsDefaults="/oce/src/StdResource";
  // Visu
  ENV.CSF_ShadersDirectory = "/oce/src/Shaders";
  ENV.CSF_MDTVTexturesDirectory = "/oce/src/Textures";
  // OCAF
  ENV.CSF_PluginDefaults="/oce/src/StdResource";
  ENV.CSF_XCAFDefaults="/oce/src/StdResource";
  ENV.CSF_TObjDefaults="/oce/src/StdResource";
  ENV.CSF_StandardDefaults="/oce/src/src/StdResource";
  ENV.CSF_StandardLiteDefaults="/oce/src/src/StdResource";
  // Dataexchange
  ENV.CSF_XSMessage	= "/oce/src/XSMessage";
  ENV.CSF_SHMessage	= "/oce/src/SHMessage";
  ENV.CSF_STEPDefaults="/oce/src/XSTEPResource";
  ENV.CSF_IGESDefaults="/oce/src/XSTEPResource";
  //ENV.CSF_ResourceVerbose = 1;
}
