$(function() {
var dps = [];
for(var i=0;i<10;i+=0.01) {
dps.push(Math.sin(i));
}

var chart = $('#chartContainer');
chart.attr('width',$(window).width());
var smoothie = new SmoothieChart({ minValue: -180, maxValue: 180});
smoothie.streamTo(chart[0]);

var rollseries = new TimeSeries();
var pitchseries = new TimeSeries();
var yawseries = new TimeSeries();
smoothie.addTimeSeries(rollseries,{strokeStyle:'red'});
smoothie.addTimeSeries(pitchseries,{strokeStyle:'green'});
smoothie.addTimeSeries(yawseries,{strokeStyle:'blue'});

  function clamp(v) {
    while(v < 0) {
      v = 360 + v;
    }
    while(v > 360) {
      v = v - 360;
    }
    return v;
  }

    var socket = io();
    var tosub = ["AttitudeState","ManualControlCommand","StabilizationDesired"];
    socket.on('connect', function() {
      console.log("Socket connected");
      socket.emit("subscribe", tosub);
    });
    _.each(tosub, function(s) {
      socket.on(s, function(data) {
        $('#' + data.name).html(JSON.stringify(data,null,2));
      });
    });
    socket.on("AttitudeState", function(data) {
      var t = new Date().getTime();
      rollseries.append(t,data.Roll);
      pitchseries.append(t,data.Pitch);
      yawseries.append(t,data.Yaw);
    });
});
