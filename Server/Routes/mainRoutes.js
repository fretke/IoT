const express = require("express");
const router = express.Router();
const mainController = require("../Controller/mainController");

router.get("/", mainController.getHomePage);

router.post("/logIn", mainController.logIn);

router.post("/logInCookie", mainController.logInCookie);

router.post("/updateDevice", mainController.updateDevice);

router.post("/addDevice", mainController.addDevice);

router.post("/deleteDevice", mainController.deleteDevice);

router.post("/addServo", mainController.addServo);

router.post("/deleteServo", mainController.deleteServo);

router.post("/updateServo", mainController.updateServo);

router.post("/saveNewSequence", mainController.saveNewSequence);

router.post("/deleteSequence", mainController.deleteSequence);

module.exports = router;
