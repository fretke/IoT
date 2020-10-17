const socket = require("socket.io");

class WsManager {

    constructor(server) {
        this._socket = socket(server);
        this._property = "property";
    }

    initService(){
        console.log(this._property, "initService");
        this._socket.sockets.on("connection", this.setConnection.bind(this))
    }

    getSocket(){
        return this._socket;
    }

    setConnection(socket){
        console.log(this._property, "socket on bulb");
        socket.on("room", (room) => {
            console.log("trying to enter room");
            socket.room = room;
            socket.join(room);
        });

        socket.on("updateBulb", (data) => {
            console.log("updatingBulb");
            let ledStatus = "off";
            if (data) ledStatus = "on";
            socket.broadcast.to(socket.room).emit("led", { ledIsOn: ledStatus });
            this._socket.to(socket.room).emit("updateStarted");
        });

        socket.on("updateServo", (data) => {
            socket.broadcast.to(socket.room).emit("Servo", data);
            this._socket.to(socket.room).emit("updateStarted");
        });

        socket.on("taskCompleted", (data) => {
            console.log(data, " <- data from controller");
            let response = null;
            data === "success" ? (response = true) : (response = false);
            socket.to(socket.room).emit("controllerDone", { status: data });
        });

        socket.on("excecuteSequence", (data) => {
            console.log(data, "server received sequence data from socket");
            socket
                .to(socket.room)
                .emit("playSequence", { numberOfMoves: data.length, data });
            this._socket.to(socket.room).emit("updateStarted");
        });

        socket.on("servoPos", (res) => {
            console.log(res.data, "data received from controller");
            const allMotors = ["firstServo", "secondServo", "thirdServo"];
            const formated = res.data.split(".").map((data, index) => {
                const arr = data.split(",");
                return {
                    name: allMotors[index],
                    pos: arr[1],
                    speed: arr[0],
                };
            });
            socket
                .to(socket.room)
                .emit("controllerDone", { status: true, data: formated });
        });
    }
}

module.exports = WsManager;
