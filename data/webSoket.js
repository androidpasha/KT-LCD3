//let gateway = `ws://${window.location.hostname}:81/`;
let gateway = `ws://192.168.4.1:81/`;
let websocket;
window.addEventListener('load', onLoadWindow);

function initWebSocket() {
    websocket = new WebSocket(gateway);
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onClose(event) {
    setTimeout(initWebSocket, 100);
}

function onMessage(event) {

    let jsonParseEvent = JSON.parse(event.data);
    console.log(jsonParseEvent);
    fillKeyInData(DATA, jsonParseEvent);

    function fillKeyInData(DATA, JsonObject) {
        for (let key in JsonObject) {
            if (typeof JsonObject[key] === "object") {
                fillKeyInData(DATA[key], JsonObject[key]);
            } else {
                DATA[key] = JsonObject[key];
            }
        }
    }
}

function onLoadWindow(event) {
    initWebSocket();
}

function sendData(sendString) {
    // let payload = JSON.stringify(data);
    websocket.send(sendString);
}