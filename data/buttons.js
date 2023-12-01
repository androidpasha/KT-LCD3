document.querySelector("#ButtonMinus").addEventListener("click", () => {
    if (DATA.PAS > 0)
    websocket.send('{"PASS":' + (DATA.PAS - 1) + '}');
});

document.querySelector("#ButtonPlus").addEventListener("click", () => {
    if (DATA.PAS < 5)
    websocket.send('{"PASS":' + (DATA.PAS + 1) + '}');
});

document.querySelector("#ButtonCruise").addEventListener("click", () => {
    DATA.ledsInTablo.LedCruise = !DATA.ledsInTablo.LedCruise;
    websocket.send('{"Cruise":' + DATA.ledsInTablo.LedCruise + '}');
});

document.querySelector("#Button6km").addEventListener("click", () => {
    DATA.Button6km =  !DATA.Button6km;// Инвертируем значение
    websocket.send('{"WalkMode":' + DATA.Button6km + '}');
});

setTimeout(function(){
	document.querySelector('#settingsButton').style.display = 'none';
}, 15000);

// function displayPAS() {
//     document.querySelector("#PasText").textContent = DATA.PAS;
// }