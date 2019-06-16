// Copyright (c) 2019 Gabriel Cuvillier - Continuation Labs

var Module = typeof Module !== 'undefined' ? Module : {};

if (Module['preRun'] instanceof Array) {
  Module['preRun'].push(oce_setup_environment);
} else {
  Module['preRun'] = [oce_setup_environment];
}

function oce_setup_environment() {
  console.info("Setup OCE in-memory filesystem root = /oce");
  FS.createPath('/', 'oce', true, true);

  console.info("Setup OCE environment variables:");
  ENV.CASROOT = "/oce";
  ENV.CSF_LANGUAGE = "us";
  console.info("CSF_LANGUAGE="+ENV.CSF_LANGUAGE);
  console.info("CASROOT="+ENV.CASROOT);
  // Units
  ENV.CSF_UnitsLexicon = "/oce/src/UnitsAPI/Lexi_Expr.dat";
  ENV.CSF_UnitsDefinition	= "/oce/src/UnitsAPI/Units.dat";
  ENV.CSF_CurrentUnitsDefaults =    "/oce/src/StdResource";
  ENV.CSF_MDTVCurrentUnitsDefaults="/oce/src/StdResource";
  console.info("CSF_UnitsLexicon="+ENV.CSF_UnitsLexicon);
  console.info("CSF_UnitsDefinition="+ENV.CSF_UnitsDefinition);
  console.info("CSF_CurrentUnitsDefaults="+ENV.CSF_CurrentUnitsDefaults);
  console.info("CSF_MDTVCurrentUnitsDefaults="+ENV.CSF_MDTVCurrentUnitsDefaults);
  // Visu
  ENV.CSF_ShadersDirectory = "/oce/src/Shaders";
  ENV.CSF_MDTVTexturesDirectory = "/oce/src/Textures";
  console.info("CSF_ShadersDirectory="+ENV.CSF_ShadersDirectory);
  console.info("CSF_MDTVTexturesDirectory="+ENV.CSF_MDTVTexturesDirectory);
  // OCAF
  ENV.CSF_PluginDefaults="/oce/src/StdResource";
  ENV.CSF_XCAFDefaults="/oce/src/StdResource";
  ENV.CSF_TObjDefaults="/oce/src/StdResource";
  ENV.CSF_StandardDefaults="/oce/src/src/StdResource";
  ENV.CSF_StandardLiteDefaults="/oce/src/src/StdResource";
  console.info("CSF_PluginDefaults="+ENV.CSF_PluginDefaults);
  console.info("CSF_XCAFDefaults="+ENV.CSF_XCAFDefaults);
  console.info("CSF_TObjDefaults="+ENV.CSF_TObjDefaults);
  console.info("CSF_StandardDefaults="+ENV.CSF_StandardDefaults);
  console.info("CSF_StandardLiteDefaults="+ENV.CSF_StandardLiteDefaults);
  // Dataexchange
  ENV.CSF_XSMessage	= "/oce/src/XSMessage";
  ENV.CSF_SHMessage	= "/oce/src/SHMessage";
  ENV.CSF_STEPDefaults="/oce/src/XSTEPResource";
  ENV.CSF_IGESDefaults="/oce/src/XSTEPResource";
  console.info("CSF_XSMessage="+ENV.CSF_XSMessage);
  console.info("CSF_SHMessage="+ENV.CSF_SHMessage);
  console.info("CSF_STEPDefaults="+ENV.CSF_STEPDefaults);
  console.info("CSF_IGESDefaults="+ENV.CSF_IGESDefaults);

  //ENV.CSF_ResourceVerbose = 1;
}
