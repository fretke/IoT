const User = require("../Models/user");

exports.getLEDStatus = async (req, res, next) => {
  try {
    const currentUser = await User.findOne({ email: req.params.userEmail });
    if (currentUser) {
      let response = convertResponse(currentUser.IoT);

      res.send(JSON.stringify({ ...response, success: "yes" }));
    } else {
      res.send(JSON.stringify({ success: "no" }));
      console.log("no such user");
    }
  } catch (err) {
    console.log(err, "error in retrieving data for esp8266");
  }
  console.log(req.params.userEmail, "userEmail");
};

exports.getServoInfo = async (req, res) => {
  const currentUser = await User.findOne({ email: req.params.userEmail });
  const {servos} = currentUser.IoT;
  res.send(JSON.stringify({
    servoQty: servos.length,
    servos
  }))
}

exports.getSwitchInfo = async (req, res) => {
  console.log("received request for switch info " + req.params.userEmail);
  const currentUser = await User.findOne({ email: req.params.userEmail });
  const {switches} = currentUser.IoT;
  console.log(switches, "all switches");
  res.send(JSON.stringify({
    qty: switches.length,
    switches
  }));
}

const convertResponse = (IoT) => {
  let converted = {};
  for (item in IoT) {
    if (item === "servos") {
      IoT[item].forEach((servo, index) => {
        converted = {
          ...converted,
          [servo.name]: servo.pos,
          [servo.name + "Speed"]: servo.speed,
        };
      });
    } else if (IoT[item] === true) {
      converted = {
        ...converted,
        [item]: "on",
      };
    } else {
      converted = {
        ...converted,
        [item]: "off",
      };
    }
  }
  return converted;
};
