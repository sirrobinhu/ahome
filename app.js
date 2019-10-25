const mariadb = require('mariadb/callback');
const SerialPort = require('serialport');
const colors = require('colors');
const moment = require('moment');
// const port = new SerialPort('/dev/ttyUSB0', { baudRate: 9600,  parser: new SerialPort.parsers.Readline("\n") });
const port = new SerialPort('COM5', {
  baudRate: 9600,
  parser: new SerialPort.parsers.Readline("\n")
});
const http = require('http');
const express = require('express');
const app = express();
const config = require('config');
const webConfig = config.get('SmartHome.webConfig');
const conn = mariadb.createConnection({
  host: '192.168.1.10',
  user: 'pi',
  password: '1azIst3n',
  database: 'home_control',
  connectTimeout: 10000
});
conn.connect(err => {
  if (err) {
    console.log("not connected due to error: " + err);
  } else {
    console.log("connected ! connection id is " + conn.threadId);
    getDevices();
  }
});

let bufferData = "";
let incomingObj = {};
let lastDate = null;
let devices = [];
getDevices = function () {
  conn.query("SELECT * FROM devices", (err, rows, meta) => {
    if (err) {
      console.log('Can\'t connect to the database'.red);
      return;
    };
    rows.forEach(row => {
      console.log(row);
      const device = devices.find(d => d.id === row['id']);
      if (!device)
        devices.push(row);
    });
  });
};

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
    if (err) {
      console.log('Can\'t connect to the database'.red);
      return;
    };
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

port.on('data', function (data) {
  bufferData += data.toString('utf8');
  if (bufferData.endsWith('\n') && bufferData.length > 1) {
    try {
      incomingObj = JSON.parse(bufferData);
      // console.log('Incoming:'.magenta, incomingObj);
      let sender = incomingObj['sender'];
      saveSensorValue(incomingObj);
      bufferData = "";
    } catch (err) {
      bufferData = "";
    }
  }
});

//Set time once in a day
setInterval(function () {  
  let receivers = devices.filter(obj => {
    return obj.type === 7;
  });
  receivers.forEach(function (element) {
    let cmd = `TM:${element.did},${getTimeString()}`;
    port.write(cmd);
    console.log('Time sent to: ' + element.did);

    cmd = `DT:${element.did},${getDateString()}`;
    port.write(cmd);
    console.log('Date sent to: ' + element.did);
  });
}, 3.6e+6);

//Send out temperature to clocks
setInterval(function () {
  let receivers = devices.filter(obj => {
    return obj.type === 7;
  });
  // console.log(receivers);
  conn.query("SELECT * FROM sensor_values WHERE TYPE = 1 ORDER BY date DESC LIMIT 1", (err, rows, meta) => {
    if (err) {
      console.log('Can\'t connect to the database'.red);
      return;
    };
    let f = new Date();
    f.addHours(1);

    rows.forEach(function(row) {
      var r = row;
      var sendingData;
      if (f > row.data) {
        sendingData = `TEMP:${element.did},--`
      } else {        
        receivers.forEach(function (element) {    
          sendingData = `TEMP:${element.did},${r.value}`                    
        });        
      }
      port.write(sendingData);
      console.log('Sent out: '.cyan + sendingData.white);
    });
  });
  
 
}, 600000);

getDateString = function () {
  let dt = new Date();
  let year = dt.getFullYear().toString().slice(2);
  let month = twoDigitString(dt.getMonth() + 1);
  let day = twoDigitString(dt.getDate());
  return `${year}${month}${day}`;
};

getTimeString = function () {
  let dt = new Date();
  let hour = twoDigitString(dt.getHours());
  let minutes = twoDigitString(dt.getMinutes());
  return `${hour}${minutes}`;
};

twoDigitString = function (num) {
  if (num > 10)
    return num.toString();
  return `0${num}`;
};

saveSensorValue = function (incomingObj) {
  let sensorValue = incomingObj['value'];

  let formatteddate = moment().format('YYYY-MM-D HH:mm:ss');
  let date = new Date;
  try {
    let q = `INSERT INTO sensor_values (device_id, value, type, date) VALUES ((SELECT id FROM devices WHERE did = "${incomingObj.sender}"), ${sensorValue}, ${incomingObj.cmd}, '${formatteddate}')`;

    if (lastDate === null || lastDate < date.getTime()) {
      lastDate = date.getTime() + 10 * 60 * 1000;
      console.log('Save'.yellow, q);
      conn.query(q, (err, rows, meta) => {
        if (err) {
          console.log('Couldn\'t save the record: '.red + q.white);          
          return;
        }
      });
    }
  } catch (error) {
    console.log('Couldn\'t save the record: '.red + q.white);
  }
}

Date.prototype.addHours= function(h){
  this.setHours(this.getHours()+h);
  return this;
}