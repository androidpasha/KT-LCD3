setTimeout(() => {// BMW стрелка
    DATA.Velocity.Current_speed = 45;
    arrow.style.transitionDuration = "1s";
    setTimeout(() => {
        DATA.Velocity.Current_speed = 0;
        arrow.style.transitionDuration = "0.5s";
    }, 200);
}, 1200);



//Узнаю центр вращения стрелок. Беру его из центра окружности над стрелкой
x = ArrowCircleCenter.getAttribute('cx');
y = ArrowCircleCenter.getAttribute('cy');
//Вращение стрелок и отображение одометров и батареи
setInterval(() => {
    //Speed arrow 
    arrow.setAttribute('transform', `translate(0,0) rotate(${DATA.Velocity.Current_speed * 6} ` + x + " " + y + ")");
    //AVG arrow
    arrow_AVG.setAttribute('transform', `translate(0,0) rotate(${DATA.Velocity.avg * 6} ` + x + " " + y + ")"); //7370" cy="5539//4475,8418 6161,6595 7847,4772 7984,4908 8121,5045 6298,6731
    //Отображение одометров
    km = DATA.Odometer.general - DATA.Odometer.daily;
    km = Math.floor(km * 1000) / 1000;
    DayliOdometrText.textContent = ('00' + km.toFixed(3)).slice(-7);
    km = DATA.Odometer.general;
    km = Math.floor(km);
    OdometrText.textContent = ('0000' + km.toFixed(0)).slice(-5);
    //Отображение лампочек круиза, чек и т.д
    for (key in DATA.ledsInTablo) {
        (DATA.ledsInTablo[key] < 1) ? opacity = 0.1 : opacity = 1;
        document.getElementById(key).setAttribute('opacity', opacity);
    };
    // Отображение ошибки Check
    error = document.getElementById('LedCheckText');
    (DATA.ledsInTablo.LedCheck >= 1) ?
        error.textContent = '0' + DATA.ledsInTablo.LedCheck + 'InFo' :
        error.textContent = 'CHECK';

    //Круглый прогрессбар батареи
    Bat = document.getElementById('BattaryProgress');
    PercentToBatteryCircle = LSegment / 100 * DATA.Power.BatteryPercent;
    Bat.setAttribute("stroke-dasharray", PercentToBatteryCircle + ", " + L);
    // Рассчитываю цвет прогресс бара батареи в зависимости от %
    g = Math.floor(DATA.Power.BatteryPercent * 8);
    r = 510 - g;
    (g > 255) ? g = 255 : [];
    (r > 255) ? r = 255 : (r < 0) ? r = 0 : [];
    b = 0;
    color = "#" + (1 << 24 | r << 16 | g << 8 | b).toString(16).slice(1);

    Bat.style.stroke = color;

    //Круиз
    (DATA.ledsInTablo.LedCruise) ? color = '#FA920A' : color = '#4FC419';
    document.querySelector('#ButtonCruise_circle').style.fill = color;
    //WalkcMode
    (DATA.Button6km) ? color = '#FA920A' : color = '#4FC419';
    document.querySelector('#Button6km_circle').style.fill = color;
    //PAS
    document.querySelector("#PasText").textContent = DATA.PAS;

}, interval);