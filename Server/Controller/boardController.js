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
  //   res.send(JSON.stringify({ status: "ok" }));
};

const convertResponse = (IoT) => {
  let converted = {};
  for (item in IoT) {
    if (item === "servos") {
      converted = {
        ...converted,
        [IoT[item][0].name]: IoT[item][0].pos,
        [IoT[item][0].name + "Speed"]: IoT[item][0].speed,
      };
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
