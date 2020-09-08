const express = require("express");
const router = express.Router();
const mainController = require("../Controller/mainController");

router.get("/", mainController.getHomePage);

router.post("/logIn", mainController.logIn);

router.post("/updateBulb", mainController.updateBulb);

module.exports = router;
