var uavtalk_packet = require("./uavtalk_packet");
var uavtalk_decode = require("./uavtalk_decodejson");
var fs = require('fs');
var constants = require('constants');
buffertools = require('buffertools')
buffertools.extend(); 
var _ = require('underscore');
var bufferpack = require('bufferpack');

var uavtalk_decoder = uavtalk_decode.decoder("../../build/uavobject-synthetics/json");

var sampleTxt = process.argv[2];
var stats = fs.statSync(sampleTxt)
var fileSizeInBytes = stats["size"]
var t = new Date().getTime();

function lshift(num, bits) {
  return num * Math.pow(2,bits);
}

function min(a,b) {
  if(a < b) {
    return a;
  } else {
    return b;
  }
}

function parseLog() {

if(!uavtalk_decoder.ready()) {
    console.log("JSON files may be missing, or decoded did not have enough time to parse the .json files");
    return;
}
else
{
    console.log("UAVTalk Decoder is good to go");
}

fs.open(sampleTxt, 'r', function(status, fd) {
    if (status) {
        console.log(status.message);
        return;
    }
    var data = new Buffer(fileSizeInBytes);
    data.clear();

    fs.read(fd, data, 0, fileSizeInBytes, 0, function(err, num) { 
	fs.close(fd);
    });

  var types = {
    0x0: "OBJ",
    0x1: "OBJ_REQ",
    0x2: "OBJ_ACK",
    0x3: "OBJ_ACK",
    0x4: "OBJ_NAK",
  };

  var headerbuffer = new Buffer(12);
  var headerbufferlen = 0;
  var databuffer = null;
  var datatoread = 0;
  var state = 0;

  var message = {
    type: null,
    object_id: null,
    instance_id: null,
    data: null,
    crc: null
  };

  var heard = {};

    var index = 0;
    b = Buffer(1);
    b.clear();
    b.write("<");

    while(index < fileSizeInBytes) {
      console.log("TOP state: " + state + ", index: " + index + ", fileSizeInBytes: " + fileSizeInBytes);
      if(state === 0) {
        // sync
        if(data[index] !== 0x3c) {
//          console.log("Missed sync");
          ++index;
        } else {
          headerbuffer[0] = 0x3c;
          headerbufferlen = 1;
          ++state;
          ++index;
        }
      } else if(state === 1) {
        // Read the rest of the header into the buffer
        // 10 bytes total
        var tocopy = min(fileSizeInBytes - index,10 - headerbufferlen);
        data.copy(headerbuffer,headerbufferlen,index,index + tocopy);
        headerbufferlen += tocopy;
        index += tocopy;
        if(headerbufferlen === 10) {
          // Decode the header
          var header = bufferpack.unpack("<BBHiH",headerbuffer);
          datatoread = header[2] - headerbufferlen;
          if(datatoread < 0) {
           datatoread = 0;
          }
          if(datatoread > 255) {
           datatoread = 255;
          }
          databuffer = new Buffer(datatoread);

          message.type = types[header[1] & 0x0f];
          message.object_id = header[3];
          message.instance_id = header[4];
          message.data = databuffer;
          ++state;
        }
      } else if(state === 2) {
        var tocopy = min(fileSizeInBytes - index,datatoread);
        data.copy(databuffer,databuffer.length - datatoread,index,index + tocopy);
        datatoread -= tocopy;
        index += tocopy;

        if(datatoread === 0) {
          ++state;
        }
      } else if(state === 3) {
        message.crc = data[index];

  	var data = uavtalk_decoder.decode(message);

  	if(!data) {
	  console.log("NO data");
    	  return;
  	}

      console.log(data.name);
      console.log(data);

//    console.log(headerbuffer);
//    console.log(message);

      index++;
        state = 0;
      } 
      else {
        throw("Unknown state");
      }
      if(index > fileSizeInBytes) {
        throw("SOMETHING IS WRONG");
      }

    }
});

}

// Allow parser to load JSON files before attempting to parse. 
setTimeout(parseLog, 1000); // give decoder time to load the UAVOfiles

