const interval = 100;
let useGPS = false;
let DATA = { //Объект с данными таблицы 
    Velocity: {
        Current_speed: 0,
        avg: 0,
        Vmax: 0,
    },
    driveTime: {
        drive: 0,
        all: 0,
    },
    Odometer: {
        general: 0,
        daily: 0,
        afterPowerOn: 0,
        afterCharging: 0,
        afterService: 0,
        afterLubrication: 0,
    },
    Power: {
        momentary: 0,
        sum: 0,
        current: 0,
        voltage: 0,
        BatteryPercent: 0,
    },
    ledsInTablo: { // Названия строго совпадают с id лампочек! 0.1 выкл, 1 вкл. Это прозрачность
        LedCruise: 0,
        LedBrake: 0,
        LedCheck: 0,
        LedAkkumulator: 0,
        LedLight: 0,
        LedTemperature: 0,
    },
    PAS: 0,
    Button6km: 0,
    assist:0,
    throttle:0,
};