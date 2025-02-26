const express = require('express');
const app = express();
const http = require('http');
const server = http.createServer(app);
const { Server } = require("socket.io");
const io = new Server(server);
require('events').EventEmitter.defaultMaxListeners = 15;



var fs = require('fs');
app.get('/', (req, res) => {
  //res.send('<h1>Hello world</h1>');
  res.sendFile(__dirname + '/index.html');
});


const msg = fs.readFileSync('flight 01.txt').toString().split("\n");
//console.log(msg);

//io.listen(app);

//console.log(fr)
//io.emit(fr);

var x = 0; 

  //io.emit( 'data', fr);



io.on('connection', (socket) => {
  console.log('a user connected');
  //console.log(fr);7
 // setInterval(function() {
    var fr = "";
    fr = msg[x];
    console.log(fr);
    
    
    
    io.emit( 'data', fr);
    x++;
  //} ,2000);
  

});

// fs.writeFile('Output.txt', msg[0], (err) => {
          
//   // In case of a error throw err.
//   if (err) throw err;
// });
// fs.appendFile("Output.txt", msg[1], (err) => {
//   if (err) {
//     console.log(err);
//   }
//   else {
//     // Get the file contents after the append operation
//     console.log("\nFile Contents of file after append:",
//       fs.readFileSync("example_file.txt", "utf8"));
//   }
// });






server.listen(3000);