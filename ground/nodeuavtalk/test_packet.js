var uavtalk_packet = require("./uavtalk_packet");
var net = require('net');

var cc3d_tcp = new net.Socket();
cc3d_tcp.connect(12345,"localhost", function() {
  console.log("cc3d connected to tcp gateway");
});

cc3d_tcp.on("data", uavtalk_packet.parser(function(packet) {
  console.log(packet);
}));

