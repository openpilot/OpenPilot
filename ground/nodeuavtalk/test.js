var EventEmitter = require('events').EventEmitter;
var uavtalk_packet = require("./uavtalk_packet");
var uavtalk_decode = require("./uavtalk_decodejson");
var SerialPort = require("serialport").SerialPort;
var _ = require('underscore');
var net = require('net');

var cc3d_tcp = new net.Socket();
cc3d_tcp.connect(12345,"localhost", function() {
  console.log("cc3d connected to tcp gateway");
});

var dataemitter = new EventEmitter();
function printhandler(data) {
  console.log(data);
}

var uavtalk_decoder = uavtalk_decode.decoder("../uavtalk_json");

cc3d_tcp.on("data", uavtalk_packet.parser(function(packet) {
  if(!uavtalk_decoder.ready()) {
    return;
  }
  var data = uavtalk_decoder.decode(packet);
  if(!data) {
    return;
  }
  dataemitter.emit(data.name,data);
}));


var express = require('express');
var app = express();
var http = require('http').Server(app);
var io = require('socket.io')(http);

io.on('connection', function(socket){
  console.log('a user connected');
  var subs = [];
  socket.on("subscribe", function(names) {
    console.log("Subscribing to " + names);
    _.each(names, function(n) {
      var forward = function(data) {
        socket.emit(data.name,data);
      }
      dataemitter.on(n, forward);
      subs.push([n,forward]);
    });
  });
  socket.on('disconnect', function() {
    console.log("a user disconnected");
    _.each(subs, function(s) {
      dataemitter.removeListener(s[0], s[1]);
    });
  });
});

app.use(express.static('public'));

http.listen(3000, function(){
  console.log('listening on *:3000');
});

