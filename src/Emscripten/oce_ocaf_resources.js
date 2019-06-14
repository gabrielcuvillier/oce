
var Module = typeof Module !== 'undefined' ? Module : {};

if (!Module.expectedDataFileDownloads) {
  Module.expectedDataFileDownloads = 0;
  Module.finishedDataFileDownloads = 0;
}
Module.expectedDataFileDownloads++;
(function() {
 var loadPackage = function(metadata) {

    var PACKAGE_PATH;
    if (typeof window === 'object') {
      PACKAGE_PATH = window['encodeURIComponent'](window.location.pathname.toString().substring(0, window.location.pathname.toString().lastIndexOf('/')) + '/');
    } else if (typeof location !== 'undefined') {
      // worker
      PACKAGE_PATH = encodeURIComponent(location.pathname.toString().substring(0, location.pathname.toString().lastIndexOf('/')) + '/');
    } else {
      throw 'using preloaded data can only be done on a web page or in a web worker';
    }
    var PACKAGE_NAME = '../src/Emscripten/oce_ocaf_resources.data';
    var REMOTE_PACKAGE_BASE = 'oce_ocaf_resources.data';
    if (typeof Module['locateFilePackage'] === 'function' && !Module['locateFile']) {
      Module['locateFile'] = Module['locateFilePackage'];
      err('warning: you defined Module.locateFilePackage, that has been renamed to Module.locateFile (using your locateFilePackage for now)');
    }
    var REMOTE_PACKAGE_NAME = Module['locateFile'] ? Module['locateFile'](REMOTE_PACKAGE_BASE, '') : REMOTE_PACKAGE_BASE;
  
    var REMOTE_PACKAGE_SIZE = metadata.remote_package_size;
    var PACKAGE_UUID = metadata.package_uuid;
  
    function fetchRemotePackage(packageName, packageSize, callback, errback) {
      var xhr = new XMLHttpRequest();
      xhr.open('GET', packageName, true);
      xhr.responseType = 'arraybuffer';
      xhr.onprogress = function(event) {
        var url = packageName;
        var size = packageSize;
        if (event.total) size = event.total;
        if (event.loaded) {
          if (!xhr.addedTotal) {
            xhr.addedTotal = true;
            if (!Module.dataFileDownloads) Module.dataFileDownloads = {};
            Module.dataFileDownloads[url] = {
              loaded: event.loaded,
              total: size
            };
          } else {
            Module.dataFileDownloads[url].loaded = event.loaded;
          }
          var total = 0;
          var loaded = 0;
          var num = 0;
          for (var download in Module.dataFileDownloads) {
          var data = Module.dataFileDownloads[download];
            total += data.total;
            loaded += data.loaded;
            num++;
          }
          total = Math.ceil(total * Module.expectedDataFileDownloads/num);
          if (Module['setStatus']) Module['setStatus']('Downloading data... (' + loaded + '/' + total + ')');
        } else if (!Module.dataFileDownloads) {
          if (Module['setStatus']) Module['setStatus']('Downloading data...');
        }
      };
      xhr.onerror = function(event) {
        throw new Error("NetworkError for: " + packageName);
      }
      xhr.onload = function(event) {
        if (xhr.status == 200 || xhr.status == 304 || xhr.status == 206 || (xhr.status == 0 && xhr.response)) { // file URLs can return 0
          var packageData = xhr.response;
          callback(packageData);
        } else {
          throw new Error(xhr.statusText + " : " + xhr.responseURL);
        }
      };
      xhr.send(null);
    };

    function handleError(error) {
      console.error('package error:', error);
    };
  
      var fetchedCallback = null;
      var fetched = Module['getPreloadedPackage'] ? Module['getPreloadedPackage'](REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE) : null;

      if (!fetched) fetchRemotePackage(REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE, function(data) {
        if (fetchedCallback) {
          fetchedCallback(data);
          fetchedCallback = null;
        } else {
          fetched = data;
        }
      }, handleError);
    
  function runWithFS() {

    function assert(check, msg) {
      if (!check) throw msg + new Error().stack;
    }
Module['FS_createPath']('/', 'oce', true, true);
Module['FS_createPath']('/oce', 'src', true, true);
Module['FS_createPath']('/oce/src', 'XmlOcafResource', true, true);
Module['FS_createPath']('/oce/src', 'StdResource', true, true);

    function DataRequest(start, end, audio) {
      this.start = start;
      this.end = end;
      this.audio = audio;
    }
    DataRequest.prototype = {
      requests: {},
      open: function(mode, name) {
        this.name = name;
        this.requests[name] = this;
        Module['addRunDependency']('fp ' + this.name);
      },
      send: function() {},
      onload: function() {
        var byteArray = this.byteArray.subarray(this.start, this.end);
        this.finish(byteArray);
      },
      finish: function(byteArray) {
        var that = this;

        Module['FS_createDataFile'](this.name, null, byteArray, true, true, true); // canOwn this data in the filesystem, it is a slide into the heap that will never change
        Module['removeRunDependency']('fp ' + that.name);

        this.requests[this.name] = null;
      }
    };

  
    function processPackageData(arrayBuffer) {
      Module.finishedDataFileDownloads++;
      assert(arrayBuffer, 'Loading data file failed.');
      assert(arrayBuffer instanceof ArrayBuffer, 'bad input to processPackageData');
      var byteArray = new Uint8Array(arrayBuffer);
      var curr;
      
          var compressedData = {"data":null,"cachedOffset":22112,"cachedIndexes":[-1,-1],"cachedChunks":[null,null],"offsets":[0,1194,2264,3490,3969,4534,5207,6022,7207,8030,9378,10695,11809,12764,13347,14374,15580,16355,17595,18667,19398,20096,20995,21730],"sizes":[1194,1070,1226,479,565,673,815,1185,823,1348,1317,1114,955,583,1027,1206,775,1240,1072,731,698,899,735,382],"successes":[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]}
;
          compressedData.data = byteArray;
          assert(typeof LZ4 === 'object', 'LZ4 not present - was your app build with  -s LZ4=1  ?');
          LZ4.loadPackage({ 'metadata': metadata, 'compressedData': compressedData });
          Module['removeRunDependency']('datafile_../src/Emscripten/oce_ocaf_resources.data');
    
    };
    Module['addRunDependency']('datafile_../src/Emscripten/oce_ocaf_resources.data');
  
    if (!Module.preloadResults) Module.preloadResults = {};
  
      Module.preloadResults[PACKAGE_NAME] = {fromCache: false};
      if (fetched) {
        processPackageData(fetched);
        fetched = null;
      } else {
        fetchedCallback = processPackageData;
      }
    
  }
  if (Module['calledRun']) {
    runWithFS();
  } else {
    if (!Module['preRun']) Module['preRun'] = [];
    Module["preRun"].push(runWithFS); // FS is not initialized yet, wait for it
  }

 }
 loadPackage({"files": [{"start": 0, "audio": 0, "end": 322, "filename": "/oce/src/XmlOcafResource/FILES"}, {"start": 322, "audio": 0, "end": 1802, "filename": "/oce/src/XmlOcafResource/XmlOcaf_TFunction.xsd"}, {"start": 1802, "audio": 0, "end": 3211, "filename": "/oce/src/XmlOcafResource/XmlOcaf_TDocStd.xsd"}, {"start": 3211, "audio": 0, "end": 4912, "filename": "/oce/src/XmlOcafResource/XmlOcaf_TPrsStd.xsd"}, {"start": 4912, "audio": 0, "end": 13550, "filename": "/oce/src/XmlOcafResource/XmlOcaf_TDataStd.xsd"}, {"start": 13550, "audio": 0, "end": 19156, "filename": "/oce/src/XmlOcafResource/XmlOcaf.xsd"}, {"start": 19156, "audio": 0, "end": 20570, "filename": "/oce/src/XmlOcafResource/XmlOcaf_TDataStd_Name.xsd"}, {"start": 20570, "audio": 0, "end": 23941, "filename": "/oce/src/XmlOcafResource/XmlOcaf_TNaming_NamedShape.xsd"}, {"start": 23941, "audio": 0, "end": 30396, "filename": "/oce/src/XmlOcafResource/XmlOcaf_SmallTypes.xsd"}, {"start": 30396, "audio": 0, "end": 35067, "filename": "/oce/src/XmlOcafResource/XmlOcaf_TNaming.xsd"}, {"start": 35067, "audio": 0, "end": 36405, "filename": "/oce/src/XmlOcafResource/XmlOcaf_TDF.xsd"}, {"start": 36405, "audio": 0, "end": 39931, "filename": "/oce/src/XmlOcafResource/XmlXcaf.xsd"}, {"start": 39931, "audio": 0, "end": 40614, "filename": "/oce/src/StdResource/MigrationSheet.txt"}, {"start": 40614, "audio": 0, "end": 41138, "filename": "/oce/src/StdResource/MDTV-Standard.xwd"}, {"start": 41138, "audio": 0, "end": 43146, "filename": "/oce/src/StdResource/Plugin"}, {"start": 43146, "audio": 0, "end": 44516, "filename": "/oce/src/StdResource/Standard"}, {"start": 44516, "audio": 0, "end": 44683, "filename": "/oce/src/StdResource/Standard.us"}, {"start": 44683, "audio": 0, "end": 45476, "filename": "/oce/src/StdResource/StandardLite"}, {"start": 45476, "audio": 0, "end": 46009, "filename": "/oce/src/StdResource/TObj"}, {"start": 46009, "audio": 0, "end": 47913, "filename": "/oce/src/StdResource/XCAF"}], "remote_package_size": 26208, "package_uuid": "4b79ef93-5484-4b92-a4f9-c613645dfccd"});

})();
