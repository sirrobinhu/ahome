const mariadb = require('mariadb/callback');
const conn = mariadb.createConnection({host: 'localhost', user:'phpmyadmin', password: '1azIst3n', database: 'mysql'});
conn.connect(err => {
if (err) {
    console.log("not connected due to error: " + err);
} else {
    console.log("connected ! connection id is " + conn.threadId);
}
});
conn.query("SELECT * FROM user", (err, rows, meta) => {
if (err) throw err;
console.log(rows); //[ { 'now()': 2018-07-02T17:06:38.000Z } ]
});