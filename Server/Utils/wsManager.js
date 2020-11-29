const socket = require("socket.io");

class WsManager {

    constructor(server) {
        this._socket = socket(server);
        this._property = "property";
    }

    initService(){
        this._socket.sockets.on("connection", this.setConnection.bind(this))
    }

    getSocket(){
        return this._socket;
    }

    setConnection(socket){
        socket.on("room", (room) => {
            socket.room = room;
            socket.join(room);
        });

        socket.on("UpdateDevice", (data) => {
            const ledStatus = data ? "on" : "off";
            socket.broadcast.to(socket.room).emit("led", { ledIsOn: ledStatus });
            this._socket.to(socket.room).emit("updateStarted");
        });

        socket.on("UpdateServo", (data) => {
            socket.broadcast.to(socket.room).emit("Servo", data);
            this._socket.to(socket.room).emit("updateStarted");
        });

        socket.on("LiveControl", (data) => {
            socket.broadcast.to(socket.room).emit("LiveControl", data);
        })

        socket.on("taskCompleted", (data) => {
            socket.to(socket.room).emit("updateFinished", { status: data });
        });

        socket.on("excecuteSequence", (data) => {
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
                .emit("SequenceOver", formated);
        });
    }
}

module.exports = WsManager;
