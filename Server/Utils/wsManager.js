const socket = require("socket.io");

// export enum OutgoingEvents {
//     UpdateDevice = "UpdateDevice",
//         UpdateServo = "UpdateServo",
//         LiveControl = "LiveControl",
//         executeSequence = "executeSequence"
// }
//
// export enum IncomingEvents {
//     connect = "connect",
//         onDeviceToggle = "onDeviceToggle",
//         onServoUpdate = "onServoUpdate",
//         onUpdateStarted = "onUpdateStarted",
//         onUpdateFinished = "onUpdateFinished",
//         onBusyChange = "onBusyChange",
//         notConnected = "notConnected",
//         onSequenceOver = "onSequenceOver"
// }

const inEvents = {
    updateDevice: "updateDevice",
    updateServo: "updateServo",
    liveControl: "liveControl",
    executeSequence: "executeSequence",
    taskCompleted: "taskCompleted",
    servoPos: "servoPos"
}

const outEvents = {
    onDeviceToggle: "onDeviceToggle",
    onServoUpdate: "onServoUpdate",
    onUpdateStarted: "onUpdateStarted",
    onUpdateFinished: "onUpdateFinished",
    onSequenceOver: "onSequenceOver",
    playSequence: "playSequence"

}

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

        socket.on(inEvents.updateDevice, (data) => {
            const ledStatus = data ? "on" : "off";
            socket.to(socket.room).emit(outEvents.onDeviceToggle, { ledIsOn: ledStatus });
            this._socket.to(socket.room).emit(outEvents.onUpdateStarted);
        });

        socket.on(inEvents.updateServo, (data) => {
            this._socket.to(socket.room).emit(outEvents.onServoUpdate, data);
            this._socket.to(socket.room).emit(outEvents.onUpdateStarted);
        });

        socket.on(inEvents.liveControl, (data) => {
            socket.to(socket.room).emit(inEvents.liveControl, data);
        })

        socket.on(inEvents.taskCompleted, (data) => {
            socket.to(socket.room).emit(outEvents.onUpdateFinished, { status: data });
        });

        socket.on(inEvents.executeSequence, (data) => {
            socket
                .to(socket.room)
                .emit(outEvents.playSequence, { numberOfMoves: data.length, data });
            this._socket.to(socket.room).emit(outEvents.onUpdateStarted);
        });

        socket.on(inEvents.servoPos, (res) => {
            console.log(res.data, "data received from controller");
            const allMotors = ["firstServo", "secondServo", "thirdServo"];
            const formated = res.data.split(".").map((data, index) => {
                const arr = data.split(",");
                return {
                    name: arr[0],
                    pos: arr[2],
                    speed: arr[1],
                };
            });
            socket
                .to(socket.room)
                .emit(outEvents.onSequenceOver, formated);
        });
    }
}

module.exports = WsManager;
