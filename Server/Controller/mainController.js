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
  const { email, pass } = req.body;
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
      // req.session.isLoggedIn = true;
      res.send(
        JSON.stringify({
          auth: true,
          userName: user.userName,
          userEmail: user.email,
          IoT: user.IoT,
          id: user._id,
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

exports.logInCookie = async (req, res, next) => {
  console.log(req.session, "request body");
  req.session.isLoggedIn = true;
  try {
    const user = await User.findById(req.body.userId);
    if (!user) {
      res.send(
        JSON.stringify({
          auth: false,
          message: "wrong id",
        })
      );
      return;
    }
    console.log(user, "user from cookie");
    res.send(
      JSON.stringify({
        auth: true,
        userName: user.userName,
        userEmail: user.email,
        IoT: user.IoT,
        id: user._id,
      })
    );
  } catch (err) {
    console.log(err, "error while logging in with cookie");
  }
};

exports.updateDevice = async (req, res, next) => {
  try {
    const { userEmail, status, name } = req.body;

    // const result = await User.updateOne(
    //   { email: user },
    //   { $set: { "IoT.ledIsOn": status } }
    // );

    const result = await User.updateOne(
        {email: userEmail, "IoT.switches.name": name},
        { $set: {"IoT.switches.$.state": status}}
    )

    if (result.n === 1) {
      res.send(JSON.stringify({ updated: true }));
    } else {
      res.send(JSON.stringify({ updated: false }));
    }
  } catch (err) {
    console.log(err, "error updating bulb state");
  }
};

exports.addDevice = async (req, res) => {
  const {email, device} = req.body;

  const result = await User.updateOne(
      {email},
      {$push: {"IoT.switches": device}}
  )
  if (result.n === 1) {
    res.send(JSON.stringify({ updated: true }));
  } else {
    res.send(JSON.stringify({ updated: false }));
  }
}

exports.deleteDevice = async (req, res) => {
  const {email, name} = req.body;
  console.log(req.body, "deleting...");
  const result = await User.updateOne(
      {email},
      {$pull: {"IoT.switches": {name: name}}}
  )

  if (result.n === 1) {
    res.send(JSON.stringify({ updated: true }));
  } else {
    res.send(JSON.stringify({ updated: false }));
  }
}

exports.addServo = async (req, res) => {
  const {email, servo} = req.body;
  // console.log(servo, "servo");
  // res.send("good");
  const result = await User.updateOne(
      {email},
      {$push: {"IoT.servos": servo}}
  )
  if (result.n === 1) {
    res.send(JSON.stringify({ updated: true }));
  } else {
    res.send(JSON.stringify({ updated: false }));
  }
}

exports.deleteServo = async (req, res) => {
  const {email, name} = req.body;

  const result = await User.updateOne(
      {email},
      {$pull: {"IoT.servos": {name: name}}}
  )
  if (result.n === 1) {
    res.send(JSON.stringify({ updated: true }));
  } else {
    res.send(JSON.stringify({ updated: false }));
  }
}

exports.updateServo = async (req, res, next) => {
  console.log(req.body, "updating servo");
  const { name, pos, speed, userEmail, id } = req.body;

  try {

     const result = await User.updateOne(
          { email: userEmail, "IoT.servos.name": name },
          { $set: { "IoT.servos.$.pos": pos, "IoT.servos.$.speed": speed } });

    if (result.n === 1) {
      res.send(JSON.stringify({ updated: true }));
    } else {
      res.send(JSON.stringify({ updated: false }));
    }
  } catch (err) {
    console.log(err, "error while updating servo in Server");
  }

};

exports.saveNewSequence = async (req, res, next) => {
  const { userEmail, newEntry } = req.body;
  console.log(req.body.newEntry, "savable sequence received");
  try {
    const response = await User.updateOne(
      { email: userEmail },
      { $push: { "IoT.seq": newEntry } }
    );
    if (response.nModified === 1) {
      res.send(JSON.stringify({ success: true }));
    } else {
      res.send(JSON.stringify({ success: false }));
    }
    console.log(response, "res from database");
  } catch (err) {
    console.log(err, "error saving new sequence in database");
  }
};

exports.deleteSequence = async (req, res, next) => {
  const { title, userEmail } = req.body;

  try {
    const response = await User.updateOne(
      { email: userEmail },
      { $pull: { "IoT.seq": { seqName: title } } }
    );
    if (response.nModified === 1) {
      res.send(JSON.stringify({ success: true }));
    } else {
      res.send(JSON.stringify({ success: false }));
    }
    console.log(response, "res from delete sequence in db");
  } catch (err) {
    console.log(err, "error when deleting sequence in database");
  }
};
