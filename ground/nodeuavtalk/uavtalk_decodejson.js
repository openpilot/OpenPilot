// This module decodes a discrete UAV packet.

var _ = require('underscore');
var bufferpack = require('bufferpack');

function endsWith(str,suffix) {
  return str.indexOf(suffix, str.length - suffix.length) !== -1;
};

function decoder(objpath) {
  console.log("Reading json object defs...");
  var fs = require('fs');
  var path = require('path');

  var uavobjects = {}
  var ready = false;

  fs.readdir(objpath, function(err, files){
  	if (err) throw err;

	var count = 1

  	function checkdone() {
		  count--;
		  if(count === 0) {
		    ready = true;
		  }
	}

	_.each(files, function(filename) {
		if(!endsWith(filename,".json")) {
		  return;
		}
		++count;
		var filename = path.join(objpath,filename);
		fs.readFile(filename, function(err, data){
		  var json = JSON.parse(data);
		  var unpackstr = "<"
		  _.each(json.fields, function(f) {
		    var u;
		    if(f.type === 0) {
		      // int8
		      u = "b"
		    } else if(f.type === 1) {
		      // int16
		      u = "h"
		    } else if(f.type === 2) {
		      // int32
		      u = "i"
		    } else if(f.type === 3) {
		      // uint8
		      u = "B"
		    } else if(f.type === 4) {
		      // uint16
		      u = "H"
		    } else if(f.type === 5) {
		      // uint32
		      u = "I"
		    } else if(f.type === 6) {
		      // float
		      u = "f"
		    } else if(f.type === 7) {
		      // enum
		      u = "B"
		    } else {
		      throw("Unknown field type: " + f.type);
		    }
		    u = u + "(" + f.name + ")";
		    if(f.numElements > 1) {
		      u = f.numElements.toString(10) + u;
		    }
		    unpackstr += u;
		  });
		  json.unpackstr = unpackstr;
		  uavobjects[json.object_id] = json;
		  checkdone();
		});
	});
	checkdone();

  });

function unpack_obj(obj,data) {
  var out = {};
  var unpacked = bufferpack.unpack(obj.unpackstr,data);
  if(!unpacked) {
    console.log("Couldn't unpack " + obj.name);
    return null;
  }
  return unpacked;
  _.each(obj.fields, function(f,index) {
    out[f.name] = unpacked[index];
  });
  return out;
}

  var warned = {};

  return {
    ready: function() { return ready; },
    decode: function(packet) {
    var obj = uavobjects[packet.object_id];
    if(!obj) {
      if(!warned[packet.object_id]) {
        console.log("JSON Failed to find object");
        console.log(packet);
	warned[packet.object_id] = true;
      }
      return null;
    } else {
      var objdata = unpack_obj(obj,packet.data);
      objdata.name = obj.name;
      return objdata;
    }
    }
  }
}

exports.decoder = decoder;
