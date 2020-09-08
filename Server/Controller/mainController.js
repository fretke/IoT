const bcrypt = require("bcrypt");
const User = require("../Models/user");
const myEmail = "alfredas.kiudys@gmail.com";

exports.getHomePage = async (req, res, next) => {
  //   bcrypt.hash("fretke.123", 10, async (err, hash) => {
  //     if (!err) {
  //       try {
  //         const user = await new User({
  //           userName: "Alfredas",
  //           email: "alfredas.kiudys@gmail.com",
  //           password: hash,
  //           IoT: {
  //             ledIsOn: "no",
  //           },
  //         }).save();

  //         console.log(user, "savedUser");
  //       } catch (err) {
  //         console.log(err, "error saving new user");
  //       }
  //     }
  //   });
  res.send("IoT server");
};

exports.logIn = async (req, res, next) => {
  const { email, pass } = req.body.userData;
  console.log(email, "body of the request");
  try {
    const user = await User.findOne({ email: email });
    if (!user) {
      res.send(
        JSON.stringify({
          auth: false,
          message: "no such user",
        })
      );
      return;
    }
    console.log(user, "user");
    const match = await bcrypt.compare(pass, user.password);
    if (match) {
      res.send(
        JSON.stringify({
          auth: true,
          userName: user.userName,
          userEmail: user.email,
          IoT: user.IoT,
        })
      );
    } else {
      res.send(
        JSON.stringify({
          auth: false,
          message: "incorrect password",
        })
      );
    }
  } catch (err) {
    console.log(err, "error logging in user");
  }
};

exports.updateBulb = async (req, res, next) => {
  try {
    const { user, status } = req.body;

    const result = await User.update(
      { email: user },
      { $set: { "IoT.ledIsOn": status } }
    );

    if (result.n === 1) {
      res.send(JSON.stringify({ updated: true }));
    } else {
      res.send(JSON.stringify({ updated: false }));
    }
  } catch (err) {
    console.log(err, "error updating bulb state");
  }
};
