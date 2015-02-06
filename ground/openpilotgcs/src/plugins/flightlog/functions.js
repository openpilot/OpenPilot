.pragma library

function millisToTime(ms) {
    var secs = Math.floor(ms / 1000);
    var msleft = ms % 1000;
    var hours = Math.floor(secs / (60 * 60));
    var divisor_for_minutes = secs % (60 * 60);
    var minutes = Math.floor(divisor_for_minutes / 60);
    var divisor_for_seconds = divisor_for_minutes % 60;
    var seconds = Math.ceil(divisor_for_seconds);
    return pad(hours, 2) + ":" + pad(minutes, 2) + ":" + pad(seconds, 2)  + ":" + pad(msleft, 3);
}


function microsToTime(us) {
    var ms = Math.floor(us / 1000);
    return millisToTime(ms);
}

function pad(number, length) {
    var str = '' + number;
    while (str.length < length) {
        str = '0' + str;
    }
    return str;
}
