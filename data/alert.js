//Окошки алерт для сброса показаний одометро

document.querySelector("#Odometr").addEventListener("click", () => {
    let km = parseFloat(prompt("Изменить значение общего одометра? Остальные одометры будут сброшены!", DATA.Odometer.general.toFixed(0)));

    if ((km != DATA.Odometer.general.toFixed(0)) && !isNaN(km)) { // Если значение изменилось и оно не равно NaN
        websocket.send('{"OdometerGeneral":' + km + '}');
        addEventListener("click",);
    }
});

document.querySelector("#V_max").addEventListener("click", () => {
    if (confirm("Сбросить максимальную скорость?")) {
        websocket.send('{"resetVmax":0}');
    }
});

document.querySelector("#DayliOdometr").addEventListener("click", () => {
    if (confirm("Сбросить суточный одометр?")) {
        websocket.send('{"resetOdometerDaily":0}');
    }
});

//ODO2

document.querySelector("#ODO2").addEventListener("click", () => {
    if (confirm("Зарядили батарею? Сбросить одометр 100% ?")) {
        websocket.send('{"resetOdometerAfterCharging":0}');
    }
});

document.querySelector("#ODO3").addEventListener("click", () => {
    if (confirm("Сбросить одометр после технического обслуживания?")) {
        websocket.send('{"resetOdometerAfterService":0}');
    }
});

document.querySelector("#ODO4").addEventListener("click", () => {
    if (confirm("Сбросить одометр после смазки цепи?")) {
        websocket.send('{"resetOdometerafterLubrication":0}');
    }
});

document.querySelector("#P_SUM").addEventListener("click", () => {
    if (confirm("Сбросить счетчик энергии?")) {
        websocket.send('{"resetWattMeter":0}'); 
    }
});

document.querySelector("#LedCheck").addEventListener("click", () => {
    switch (DATA.ledsInTablo.LedCheck) {
        case 0: break;
        case 1: alert('Неисправность ручки газа'); break;
        case 3: alert('Неисправность датчика холла мотора'); break;
        case 4: alert('Неисправность датчика системы PAS'); break;
        case 6: alert('Короткое замыкание в моторе или контроллере'); break;
        default: alert('Неизвестная ошибка №0' + DATA.ledsInTablo.LedCheck);
    }
});