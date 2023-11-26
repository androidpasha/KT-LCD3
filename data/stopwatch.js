const watch = document.querySelector('#Clock text');
let milliseconds = 0;
let timer;
let state = Number(localStorage.getItem('state'));// Читаю сохраненные данные из браузера
const State = { clock: 0, Stopwatch: 1, Stopwatch_pause: 2 };
let stopWatchStartMoment = new Date(localStorage.getItem('stopWatchStartMoment'));// Читаю сохраненные данные из браузера

const Button_start = () => {
	if (state === State.clock || state === State.Stopwatch_pause) {
		stopWatchStartMoment = new Date();// Определяем время запуска таймера
		localStorage.setItem('stopWatchStartMoment', stopWatchStartMoment);//Сохраняем в браузер
		stopwatch(stopWatchStartMoment);//Запускаем функцию таймера
		state = State.Stopwatch;
		localStorage.setItem('state', state);//Сохраняем состояние
	}
};

const Button_stop = () => {
	if (state === State.Stopwatch) {
		clearInterval(timer);
		state = State.Stopwatch_pause;
		localStorage.setItem('state', state);//Сохраняем состояние
	} else if (state === State.Stopwatch_pause) {
		Clock();
		state = State.clock
		localStorage.setItem('state', state);//Сохраняем состояние
	};
};

function stopwatch(stopWatchStartMoment) {
	clearInterval(timer);
	timer = setInterval(() => {
		const now = new Date();
		const stopwatch = new Date(now - stopWatchStartMoment);
		let stopwatchOption = { timeZone: "UTC", minute: '2-digit', second: "2-digit", fractionalSecondDigits: 1 }
		if (stopwatch.getUTCHours() !== 0)
			stopwatchOption.hour = '2-digit';
		watch.textContent = stopwatch.toLocaleTimeString([], stopwatchOption).replace(',', '.');
	}, 100);
}

const Clock = () => {
	clearInterval(timer);
	timer = setInterval(() => {
		const now = new Date();
		watch.textContent = now.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: "2-digit" });
	}, 100);
};


//Выбираю функцию после загрузки окна
if (state === State.clock || state === State.Stopwatch_pause) Clock();// Запуск функции часов
else stopwatch(stopWatchStartMoment); // Запуск функции секундомера

document.querySelector("#Button_play").addEventListener("click", Button_start); // Обратная функция при клике на кнопку
document.querySelector("#Button_stop").addEventListener("click", Button_stop);