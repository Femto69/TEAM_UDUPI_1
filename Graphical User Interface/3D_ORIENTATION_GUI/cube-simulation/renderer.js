const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');

const canvas = document.getElementById('renderCanvas');
const engine = new BABYLON.Engine(canvas, true);

let box;

const createScene = function() {
    const scene = new BABYLON.Scene(engine);
    const camera = new BABYLON.ArcRotateCamera("camera", -Math.PI / 2, Math.PI / 2.5, 3, new BABYLON.Vector3(0, 0, 0), scene);
    camera.attachControl(canvas, true);
    const light = new BABYLON.HemisphericLight("light", new BABYLON.Vector3(0, 1, 0), scene);
    box = BABYLON.MeshBuilder.CreateBox("box", {size: 1}, scene);
    return scene;
};

const scene = createScene();

engine.runRenderLoop(function() {
    scene.render();
});

window.addEventListener('resize', function() {
    engine.resize();
});

// Set up serial port communication
const port = new SerialPort({ path: 'COM7', baudRate: 115200 });
const parser = port.pipe(new ReadlineParser({ delimiter: '\r\n' }));

parser.on('data', (data) => {
    const [roll, pitch, yaw] = data.split(',').map(Number);
    if (box) {
        box.rotation = new BABYLON.Vector3(
            BABYLON.Tools.ToRadians(180-roll),
            BABYLON.Tools.ToRadians(yaw),
            BABYLON.Tools.ToRadians(180-pitch)
        );
    }
});