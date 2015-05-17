var SerialPort = require("serialport").SerialPort;

var cc3d_serial = new SerialPort("/dev/ttyAMA0", {
  baudrate: 57600
});


cc3d_serial.on("open", function() {
  console.log("Opened cc3d");
  cc3d_serial.on("data", function(packet) {
  });
});
