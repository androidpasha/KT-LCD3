//svg width="100%" height="100%"
svg = document.querySelector('svg');
svg.style.width = "210.5mm";
svg.style.height = "380mm";
svg.style.width = "100%";
svg.style.height = "100%";
svg.style.viewBox = "0 0 21050 38000";

//Правлю косяки Сoreldraw, центруя цифры в таблице и часы
keys = ['#Clock text', '#LedCheckText', '#VavgText', '#VmaxText', '#tDrive', '#tAll', '#odoAfterPowerOn', '#odoAfteCharging', '#odoAfterService', '#odoAfterLubrication', '#powerMomentary', '#powerSum', '#powerCurrent', '#powerVoltage', '#fatDrive', '#fatTotal', '#calDrive', '#calTotal'];//Массив id текстовых элементов таблицы

keys.forEach(function (key) {
    Element = document.querySelector(key);
    Element.setAttribute('text-anchor', 'middle');//Центровка
    let x = parseInt(Element.getAttribute('x'));// Координата х для сдвига вправо на 16 или 13%
    let scale = 12; // процент сдвига элемента вправо.
    if (key === '#tDrive') scale = 16;//Персональное значение сдвига
    if (key === '#Clock text') scale = 40;
    x += (x / 100) * scale;//добавляю % сдвига по оси х.
    Element.setAttribute('x', x);
});

//Прогресбар батареи. Убираю сегмент.
let x = ArrowCircleCenter.getAttribute('cx');
let y = ArrowCircleCenter.getAttribute('cy');
let Bat = document.querySelector('#BattaryProgress');
let r = parseInt(Bat.getAttribute('r')); //Радиус окружности
let L = Math.ceil(2 * Math.PI * r); //Длинна окружности
let LSegment = Math.floor(L * 0.75488773424456615019738176598168); //Длинна необходимого сегмента при 100% заряде
Bat.setAttribute('stroke-dasharray', LSegment + ', ' + L);
Bat.setAttribute('transform', 'rotate(134, ' + x + ', ' + y + ')');