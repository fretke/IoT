const express = require("express");
const router = express.Router();
const boardController = require("../Controller/boardController");

router.get("/lightBulb/:userEmail", boardController.getLEDStatus);

router.get("/servoInfo/:userEmail", boardController.getServoInfo);

module.exports = router;
