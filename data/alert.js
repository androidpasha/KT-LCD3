//Окошки алерт для сброса показаний одометро

document.querySelector("#Odometr").addEventListener("click", () => {
    let km = parseFloat(prompt("Змінити значення загального одометра? Інші одометри будуть скинуті!", DATA.Odometer.general.toFixed(0)));

    if ((km != DATA.Odometer.general.toFixed(0)) && !isNaN(km)) { // Если значение изменилось и оно не равно NaN
        websocket.send('{"OdometerGeneral":' + km + '}');
        addEventListener("click",);
    }
});

document.querySelector("#V_max").addEventListener("click", () => {
    if (confirm("Обнулити максимальну швидкість?")) {
        websocket.send('{"resetVmax":0}');
    }
});

document.querySelector("#DayliOdometr").addEventListener("click", () => {
    if (confirm("Обнулити добовий одометр?")) {
        websocket.send('{"resetOdometerDaily":0}');
    }
});

//ODO2

document.querySelector("#ODO2").addEventListener("click", () => {
    if (confirm("Зарядили батарею? Обнулити одометр?")) {
        websocket.send('{"resetOdometerAfterCharging":0}');
    }
});

document.querySelector("#ODO3").addEventListener("click", () => {
    if (confirm("Виконали технічне обслуговування? Обнулити одометр ТО?")) {
        websocket.send('{"resetOdometerAfterService":0}');
    }
});

document.querySelector("#ODO4").addEventListener("click", () => {
    if (confirm("Змазали ланцюг? Обнулити одометр?")) {
        websocket.send('{"resetOdometerafterLubrication":0}');
    }
});

document.querySelector("#P_SUM").addEventListener("click", () => {
    if (confirm("Зарядили батарею? Обнулити лічильник витрат електричної енергії батареї?")) {
        websocket.send('{"resetWattMeter":0}'); 
    }
});

document.querySelector("#calTotal").addEventListener("click", () => {
    if (confirm("Обнулити лічильник спалених калорій та жиру?")) {
        websocket.send('{"resetCalories":0}'); 
    }
});



document.querySelector("#LedCheck").addEventListener("click", () => {
    switch (DATA.ledsInTablo.LedCheck) {
        case 0: break;
        case 1: alert('Несправність ручки газу'); break;
        case 3: alert('Несправність датчика холу двигуна'); break;
        case 4: alert('Несправність датчика системи PAS'); break;
        case 6: alert('Коротке замикання в моторі чи контролері'); break;
        default: alert('Невідома помилка №0' + DATA.ledsInTablo.LedCheck);
    }
});