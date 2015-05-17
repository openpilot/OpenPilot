var SerialPort = require("serialport").SerialPort;

// must supply serial port name
var cc3d_serial = new SerialPort(process.argv[2], {
  baudrate: 57600
});


cc3d_serial.on("open", function() {
  console.log("Opened cc3d");
  cc3d_serial.on("data", function(packet) {
  });
});
