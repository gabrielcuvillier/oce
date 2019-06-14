
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
    var PACKAGE_NAME = '../src/Emscripten/oce_dataexchange_resources.data';
    var REMOTE_PACKAGE_BASE = 'oce_dataexchange_resources.data';
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
Module['FS_createPath']('/oce/src', 'SHMessage', true, true);
Module['FS_createPath']('/oce/src', 'XSMessage', true, true);
Module['FS_createPath']('/oce/src', 'XSTEPResource', true, true);

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
      
          var compressedData = {"data":null,"cachedOffset":56301,"cachedIndexes":[-1,-1],"cachedChunks":[null,null],"offsets":[0,865,1803,2575,3514,4468,5463,6282,7123,8060,8850,9582,10456,11247,11987,12654,13486,14541,15660,16734,17749,18457,19260,20012,20802,21552,22332,23088,23793,24407,25301,26488,27660,28890,30059,31092,31860,32576,33414,34181,34902,35744,36532,37230,37881,38755,39834,40835,41897,42698,43499,44360,45116,45792,46600,47361,48132,48768,49500,50695,51810,53019,54177,55116,55966],"sizes":[865,938,772,939,954,995,819,841,937,790,732,874,791,740,667,832,1055,1119,1074,1015,708,803,752,790,750,780,756,705,614,894,1187,1172,1230,1169,1033,768,716,838,767,721,842,788,698,651,874,1079,1001,1062,801,801,861,756,676,808,761,771,636,732,1195,1115,1209,1158,939,850,335],"successes":[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]}
;
          compressedData.data = byteArray;
          assert(typeof LZ4 === 'object', 'LZ4 not present - was your app build with  -s LZ4=1  ?');
          LZ4.loadPackage({ 'metadata': metadata, 'compressedData': compressedData });
          Module['removeRunDependency']('datafile_../src/Emscripten/oce_dataexchange_resources.data');
    
    };
    Module['addRunDependency']('datafile_../src/Emscripten/oce_dataexchange_resources.data');
  
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
 loadPackage({"files": [{"start": 0, "audio": 0, "end": 38, "filename": "/oce/src/SHMessage/FILES"}, {"start": 38, "audio": 0, "end": 5821, "filename": "/oce/src/SHMessage/SHAPE.us"}, {"start": 5821, "audio": 0, "end": 11623, "filename": "/oce/src/SHMessage/SHAPE.fr"}, {"start": 11623, "audio": 0, "end": 11697, "filename": "/oce/src/XSMessage/FILES"}, {"start": 11697, "audio": 0, "end": 37883, "filename": "/oce/src/XSMessage/IGES.fr"}, {"start": 37883, "audio": 0, "end": 69892, "filename": "/oce/src/XSMessage/XSTEP.fr"}, {"start": 69892, "audio": 0, "end": 96815, "filename": "/oce/src/XSMessage/IGES.us"}, {"start": 96815, "audio": 0, "end": 127207, "filename": "/oce/src/XSMessage/XSTEP.us"}, {"start": 127207, "audio": 0, "end": 127235, "filename": "/oce/src/XSTEPResource/FILES"}, {"start": 127235, "audio": 0, "end": 129526, "filename": "/oce/src/XSTEPResource/IGES"}, {"start": 129526, "audio": 0, "end": 131941, "filename": "/oce/src/XSTEPResource/STEP"}], "remote_package_size": 60397, "package_uuid": "02dc5c14-e571-4f58-a3ac-96d77489e315"});

})();
