// Запускаю повторяющуюся функцию отображения данных в таблице
setInterval(() => {

    VavgText.textContent = DATA.Velocity.avg.toFixed(1);
    VmaxText.textContent = DATA.Velocity.Vmax.toFixed(1);
    tDrive.textContent = formatTime(DATA.driveTime.drive);
    tAll.textContent = DATA.driveTime.all;

    km = DATA.Odometer.general - DATA.Odometer.afterPowerOn;
    km = Math.floor(km*10)/10;
    odoAfterPowerOn.textContent = km.toFixed(1);
    km = DATA.Odometer.general - DATA.Odometer.afterCharging;
    km = Math.floor(km*10)/10;
    odoAfteCharging.textContent = km.toFixed(1);
    km = DATA.Odometer.general - DATA.Odometer.afterService;
    km = Math.floor(km);
    odoAfterService.textContent = km.toFixed(0);
    km = DATA.Odometer.general - DATA.Odometer.afterLubrication;
    km = Math.floor(km);
    odoAfterLubrication.textContent = km.toFixed(0);

    powerMomentary.textContent = DATA.Power.momentary.toFixed(0);
    powerSum.textContent = DATA.Power.sum.toFixed(0);
    powerCurrent.textContent = DATA.Power.current.toFixed(1);
    powerVoltage.textContent = DATA.Power.voltage.toFixed(1);

    fatDrive.textContent = DATA.byte11CRC;
    fatTotal.textContent = DATA.byte11;
    calTotal.textContent=DATA.assist;
    calDrive.textContent=DATA.trottle;

}, interval);

function formatTime(val) {
    tmpTime = new Date(val).toLocaleString("ua-Ua", { timeZone: "UTC" });
    tmpTime = new Date(tmpTime);
    return tmpTime.toLocaleString([], { hour: '2-digit', minute: '2-digit' });
};