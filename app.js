const mariadb = require('mariadb/callback');
const SerialPort = require('serialport');
const colors = require('colors');
// const port = new SerialPort('/dev/ttyUSB0', { baudRate: 9600,  parser: new SerialPort.parsers.Readline("\n") });
const port = new SerialPort('COM5', { baudRate: 9600,  parser: new SerialPort.parsers.Readline("\n") });
const http = require('http');
const express = require('express');
const app = express();
const config = require('config');
const webConfig = config.get('SmartHome.webConfig');
const conn = mariadb.createConnection({host: '192.168.1.10', user:'pi', password: '1azIst3n', database: 'home_control'});
  conn.connect(err => {
  if (err) {
      console.log("not connected due to error: " + err);
  } else {
      console.log("connected ! connection id is " + conn.threadId);
  }
});

let bufferData = "";
let incomingObj = {};
let lastDate = null;

app.use(express.json());

app.get('/', (req, res) => {
  res.send('Hello world');
});

app.get('/temp', (req, res) => {
  res.send([1, 2, 3]);
});

app.get('/temp/:id', (req, res) => {
  const device = devices.find(d => d.id === req.params.id);
  if (!device) res.status(404).send('The device with the given id is missing');
  res.send(device);
});

app.get('/temp/:year/:month/:day', (req, res) => {
  res.send(req.query);
});

app.post('/device', (req, res) => {
  const device = {
    did: req.body.did,
    name: req.body.name,    
    description: req.body.description
  }

  let q = `INSERT INTO devices (did, name, description) VALUES ('${device.did}', '${device.name}', '${device.description}')`;
  conn.query(q, (err, rows, meta) => {
    if (err) throw err;    
  });
});

app.post('/command', (req, res) => {
  
  let cmd = `${req.body.cmd}:${req.body.did},${req.body.value}`;
  port.write(cmd);  
  console.log('Sent out: '.cyan + cmd.white);
  res.send();
});

app.listen(webConfig.port, () => console.log('Listening on port '.blue, webConfig.port.toString().cyan));

port.on('readable', function () {
  port.read();
});


// Switches the port into "flowing mode"

port.on('data', function (data) {
  bufferData += data.toString('utf8');  
  if (bufferData.endsWith('\n') && bufferData.length > 1) {    
    // console.log('bufferdata:', bufferData);
    try {
      incomingObj = JSON.parse(bufferData);
      console.log('Incoming:'.red, incomingObj);
      let sender = incomingObj['sender'];
      if(incomingObj.cmd == command.TEMP) {
        saveTemp(incomingObj);        
      }
      bufferData = "";
    }
    catch(err) {
      // console.log('Error:', err.Message);
      bufferData = "";
    }    

    conn.query("SELECT * FROM outside_temperature", (err, rows, meta) => {
      if (err) throw err;
      rows.forEach(row => {
      });
    });
  }
});

setInterval(function() { 

  // let cmd = `PULL:j6g,up`;
  // port.write(cmd);
  
  // let cmd = `TM:i4o,${getTimeString()}`;
  // port.write(cmd);

  // let cmd = `DT:i4o,${getDateString()}`;
  // port.write(cmd);
 }, 10000);
// }, 1000*60*30);

// cleanup database
setInterval(function(){
  
}, 604800000)

getDateString = function() {
  let dt = new Date();
  let year = dt.getFullYear().toString().slice(2);
  let month = twoDigitString(dt.getMonth()+1);
  let day = twoDigitString(dt.getDate());
  return `${year}${month}${day}`;
};

getTimeString = function() {
  let dt = new Date();  
  let hour = twoDigitString(dt.getHours());
  let minutes = twoDigitString(dt.getMinutes());
  return `${hour}${minutes}`;
};

twoDigitString = function (num) {  
  if(num > 10)
    return num.toString();
  return `0${num}`;
};

const command = {
  LIGHT: 0,
  TEMP: 1,
  TM: 2,
  DT: 3,
  PULL: 4
};

const devices = [
  {id: 'c5j', type:'temp_sensor', desc:'Temperature sensor on the balcony'},
  {id: 'i4o', type:'clock', desc:'Key holder with clock'},
];

saveTemp = function(incomingObj) {
  let temperature = incomingObj['value'];

        let receivers = devices.filter(obj => {
          return obj.type === 'clock';
        });

        receivers.forEach(element => {
          let sendingData = `TEMP:${element.id},${temperature}`
          port.write(sendingData);
          console.log('Sent out: '.cyan + sendingData.white);
        });
                
        let date = new Date;
        let q = `INSERT INTO outside_temperature (device_id, value, date) VALUES ('${sender}', ${temperature}, ${date.getTime()})`;
        if (lastDate === null || lastDate < date.getTime()) {
          lastDate = date.getTime() + 10 * 60 * 1000;
          console.log('Save'.yellow, q);        
          conn.query(q, (err, rows, meta) => {
            if (err){ 
              throw err; 
              console.log(err);     
            }
          });
        }
}