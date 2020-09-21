const express = require("express");
const bodyParser = require("body-parser");
const mainRoutes = require("./Routes/mainRoutes");
const boardRoutes = require("./Routes/boardRoutes");
const mongoose = require("mongoose");
const socket = require("socket.io");
require("dotenv").config();

const app = express();

app.use(function (req, res, next) {
  res.header("Access-Control-Allow-Origin", "*"); // update to match the domain you will make the request from
  res.header(
    "Access-Control-Allow-Headers",
    "Origin, X-Requested-With, Content-Type, Accept"
  );
  res.header("Access-Control-Allow-Methods", "GET, PUT, POST, DELETE");
  next();
});
app.use(express.json());
app.use(mainRoutes);
app.use("/esp8266", boardRoutes);
app.use(express.static("public"));
app.use(bodyParser.urlencoded({ extended: true }));

// app.get("/", (req, res) => {
//   res.send("IoT server");
// });
let io;
mongoose
  .connect(process.env.MONGOOSE, {
    useNewUrlParser: true,
    useUnifiedTopology: true,
  })
  .then((res) => {
    const server = app.listen(process.env.PORT, () => {
      console.log("Server is running on port " + process.env.PORT);
    });
    io = socket(server);
    io.sockets.on("connection", newConnection);
    app.set("socket", io);
  })
  .catch((err) => {
    console.log(err, "not able to connect to DB. Server not launched");
  });

const newConnection = (socket) => {
  console.log(socket.id, "new connection id");
  socket.on("room", (room) => {
    console.log("trying to enter room");
    socket.room = room;
    socket.join(room);
  });

  socket.on("updateBulb", (data) => {
    let ledStatus = "off";
    if (data) ledStatus = "on";
    socket.broadcast.to(socket.room).emit("led", { ledIsOn: ledStatus });
    io.to(socket.room).emit("updateStarted");
  });

  socket.on("updateServo", (data) => {
    socket.broadcast.to(socket.room).emit("Servo", data);
    io.to(socket.room).emit("updateStarted");
  });

  socket.on("taskCompleted", (data) => {
    console.log(data, " <- data from controller");
    let response = null;
    data === "success" ? (response = true) : (response = false);
    socket.to(socket.room).emit("controllerDone", { status: data });
  });

  socket.on("excecuteSequence", (data) => {
    console.log(data, "server received sequence data from socket");
    socket
      .to(socket.room)
      .emit("playSequence", { numberOfMoves: data.length, data });
    io.to(socket.room).emit("updateStarted");
  });

  socket.on("servoPos", (res) => {
    console.log(res.data, "data received from controller");
    const allMotors = ["firstServo", "secondServo", "thirdServo"];
    const formated = res.data.split(".").map((data, index) => {
      const arr = data.split(",");
      return {
        name: allMotors[index],
        pos: arr[1],
        speed: arr[0],
      };
    });
    socket
      .to(socket.room)
      .emit("controllerDone", { status: true, data: formated });
  });
};
