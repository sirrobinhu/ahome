const SerialPort = require('serialport');
const port = new SerialPort('/dev/ttyUSB0', { baudRate: 9600,  parser: new SerialPort.parsers.Readline("\n") });
var bufferData = "";
// The open event is always emitted
/*port.on('open', function() {
  // open logic
 console.log('open');

})*/

port.on('readable', function () {
  //console.log('Data:', port.read().toString('utf8'));
  port.read();
})

// Switches the port into "flowing mode"
port.on('data', function (data) {
  bufferData += data.toString('utf8');
  //console.log('Data:', bufferData);
  if (bufferData.endsWith('}')) {
    console.log('Data:', bufferData);
    bufferData = "";
  }
})

setInterval(
    function() {
      port.write('hello just sent');
    }, 2000);

