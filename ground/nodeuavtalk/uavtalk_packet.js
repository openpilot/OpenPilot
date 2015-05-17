// This library parses raw uavtalk packets from given data.
// The higher level system is responsible for receiving data over
// a stream,file,whatever, and providing that data to this module
// which will then decode the raw packets and call the callback
// function when a full packet has been received.

var _ = require('underscore');
var bufferpack = require('bufferpack');

  var types = {
    0x0: "OBJ",
    0x1: "OBJ_REQ",
    0x2: "OBJ_ACK",
    0x3: "OBJ_ACK",
    0x4: "OBJ_NAK",
  };

function parsernew(callback) {
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
  return function(data) {
    var index = 0;
    var datalen = data.length;

    while(index < datalen) {
      //console.log("TOP state: " + state + ", index: " + index + ", datalen: " + datalen);
      if(state === 0) {
        // sync
        if(data[index] !== 0x3c) {
	  console.log("Missed sync");
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
	var tocopy = min(datalen - index,10 - headerbufferlen);
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
        var tocopy = min(datalen - index,datatoread);
        data.copy(databuffer,databuffer.length - datatoread,index,index + tocopy);
	datatoread -= tocopy;
	index += tocopy;

	if(datatoread === 0) {
	  ++state;
	}
      } else if(state === 3) {
        message.crc = data[index];
	callback(message);
	index++;
	state = 0;
      } else {
        throw("Unknown state");
      }
      if(index > datalen) {
        throw("SOMETHING IS WRONG");
      }
    }
  }
}

function parserold(callback) {
  var state = 0;
  var message = {
    type: null,
        length: null,
        object_id: null,
  instance_id: null,
  timestamp: null,
  data: null,
  crc: null
  };
  var types = {
    0x0: "OBJ",
    0x1: "OBJ_REQ",
    0x2: "OBJ_ACK",
    0x3: "OBJ_ACK",
    0x4: "OBJ_NAK",
  };
  return function(data) {
    var index = 0;
    var len = data.length;
    while(index < len) {
      var byte = data[index] & 0xff;
      if(state === 0) {
        // waiting for sync.
        if(byte !== 0x3c) {
          ++index;
	  state = 0;
          console.log("Missed sync");
          continue;
        }
      } else if(state === 1) {
        // Getting message type
        message.type = types[byte & 0x0f];
        if(!message.type) {
          console.log("Unknown message type " + byte.toString(16));
	  ++index;
	  state = 0;
	  continue;
        }
	if(byte & 0x80) {
	  throw("Didn't expect a timestamped object");
	  ++index;
	  state = 0;
	  continue;
	}
      } else if(state === 2) {
        // len byte 1
        message.length = byte;
      } else if(state === 3) {
        // len byte 2
        message.length += lshift(byte,8);
        if(message.length < 10) {
            message.data_length = 0;
        } else {
            message.data_length = message.length - 10;
        }
      } else if(state === 4) {
        // object id 0
        message.object_id = byte;
      } else if(state === 5) {
        // object id 1
        message.object_id += lshift(byte,8);
      } else if(state === 6) {
        // object id 2
        message.object_id += lshift(byte,16);
      } else if(state === 7) {
        // object id 3
        message.object_id += lshift(byte,24);
      } else if(state === 8) {
        // object id 3
        message.instance_id = byte;
      } else if(state === 9) {
        // object id 3
        message.instance_id += lshift(byte,8);
        message.data = new Buffer(message.data_length);
	message.data_index = 0;
      } else if(state === 10) {
        // Data
	// Copy as much as we can in one fell swoop
	var tocopy = min(len-index,message.data_length);
	data.copy(message.data,message.data_index,index,index + tocopy);
	message.data_length -= tocopy;
	message.data_index += tocopy;
	index += tocopy;
	if(message.data_length === 0) {
          callback(message);
          state = 0;
	  ++index;
	}
	continue;
      }
      ++state;
      ++index;
    }
  }
}

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

exports.parser = parsernew;
