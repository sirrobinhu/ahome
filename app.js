const mariadb = require('mariadb/callback');
const SerialPort = require('serialport');
// const port = new SerialPort('/dev/ttyUSB0', { baudRate: 9600,  parser: new SerialPort.parsers.Readline("\n") });
const port = new SerialPort('COM5', { baudRate: 9600,  parser: new SerialPort.parsers.Readline("\n") });
let bufferData = "";
let incomingObj = {};
let lastDate = null;

port.on('readable', function () {
  port.read();
})

const conn = mariadb.createConnection({host: '192.168.1.10', user:'pi', password: '1azIst3n', database: 'home_control'});
  conn.connect(err => {
  if (err) {
      console.log("not connected due to error: " + err);
  } else {
      console.log("connected ! connection id is " + conn.threadId);
  }
});

// Switches the port into "flowing mode"

port.on('data', function (data) {
  bufferData += data.toString('utf8');
  if (bufferData.endsWith('\n') && bufferData.length > 1) {
    console.log('Data:', bufferData);
    try {
      incomingObj = JSON.parse(bufferData);
      bufferData = "";
      
      let id = incomingObj['id'];
      let temperature = incomingObj['temperature'];
      let date = new Date;    

      let q = `INSERT INTO outside_temperature (device_id, value, date) VALUES ('${id}', ${temperature}, ${date.getTime()})`;
      if (lastDate === null || lastDate < date.getTime()) {
        lastDate = date.getTime() + 10 * 60 * 1000;
        console.log(`Save ${q}`);        
        conn.query(q, (err, rows, meta) => {
          if (err){ 
            throw err; 
            console.log(err);     
          }

        });
      }
      bufferData = "";
    }
    catch(err) {
      console.log('Error:', err.Message);
      bufferData = "";
    }
    
    

    conn.query("SELECT * FROM outside_temperature", (err, rows, meta) => {
      if (err) throw err;
      rows.forEach(row => {
      });
    });
  }
})

setInterval(function() {  
  console.log(`Time ${getDateString()} has been sent `);
  port.write(`time:${getDateString()}`);
}, 1000*60*30);

getDateString = function() {
  let dt = new Date();
  let year = dt.getFullYear().toString();
  let month = twoDigitString(dt.getMonth());
  let day = twoDigitString(dt.getDate());
  let hour = twoDigitString(dt.getHours());
  let minutes = twoDigitString(dt.getMinutes());
  return `${year}${month}${day}${hour}${minutes}`;
}

twoDigitString = function (num) {  
  if(num > 10)
    return num.toString();
  return `0${num}`;
}