const express = require("express");
const session = require("express-session");
const bodyParser = require("body-parser");
const mainRoutes = require("./Routes/mainRoutes");
const boardRoutes = require("./Routes/boardRoutes");
const mongoose = require("mongoose");
const cors = require("cors");
require("dotenv").config();

const WsManager = require("./Utils/wsManager")

const app = express();

app.use(
  session({
    secret: "random text",
    resave: false,
    saveUninitialized: false,
    cookie: {
      // httpOnly: true,
      maxAge: 3600000,
      // secure: true,
    },
  })
);
app.use(
  cors({
    origin: true,
    credentials: true,
    exposedHeaders: ["set-cookie"],
  })
);

app.use(express.json());
app.use(bodyParser.urlencoded({ extended: true }));
// app.use(bodyParser.json());
app.use(mainRoutes);
app.use("/esp8266", boardRoutes);
app.use(express.static("public"));

mongoose
  .connect(process.env.MONGOOSE, {
    useNewUrlParser: true,
    useUnifiedTopology: true,
  })
  .then((res) => {
    const server = app.listen(process.env.PORT, () => {
      console.log("Server is running on port " + process.env.PORT);
    });

    const wsManager = new WsManager(server);
    wsManager.initService();

    app.set("socket", wsManager.getSocket());
  })
  .catch((err) => {
    console.log(err, "not able to connect to DB. Server not launched");
  });
