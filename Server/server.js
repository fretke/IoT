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

mongoose
  .connect(process.env.MONGOOSE, {
    useNewUrlParser: true,
    useUnifiedTopology: true,
  })
  .then((res) => {
    const server = app.listen(process.env.PORT, () => {
      console.log("Server is running on port " + process.env.PORT);
    });
    const io = socket(server);
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
  });

  socket.on("updateServo", (data) => {
    socket.broadcast.to(socket.room).emit("Servo", data);
  });
};
