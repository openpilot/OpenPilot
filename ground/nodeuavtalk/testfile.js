var uavtalk_packet = require("./uavtalk_packet");
var uavtalk_decode = require("./uavtalk_decodejson");
var net = require('net');
var fs = require('fs');

var uavtalk_decoder = uavtalk_decode.decoder("../../build/uavobject-synthetics/json");

var heard = {};

var cc3d_file = fs.createReadStream(process.argv[2]);

cc3d_file.on("data", uavtalk_packet.parser(function(packet) {
  if(!uavtalk_decoder.ready()) {
    return;
  }
  var t = new Date();
  var data = uavtalk_decoder.decode(packet);
  if(!data) {
    return;
  }
  var info = heard[data.name];
  if(!info) {
    info = {
      last: t,
      count: 0
    }
    heard[data.name] = info;
  }
  info.count++;
  var diff = t - info.last;
  if(diff > 1000) {
    var hz = info.count / (diff / 1000.0);
    console.log(data.name + ": " + hz + "Hz");
    console.log(data);
    info.count = 0;
    info.last = t;
  }
}));
