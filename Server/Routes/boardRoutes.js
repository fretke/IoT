const express = require("express");
const router = express.Router();
const boardController = require("../Controller/boardController");

router.get("/lightBulb/:userEmail", boardController.getBulbStatus);

module.exports = router;