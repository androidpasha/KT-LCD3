let interval = 1000;
const DATA = { //Объект с данными таблицы 
    Velocity: {
        Current_speed: 0,
        avg: 0,
        Vmax: 0,
    },
};
x = 0;

setInterval(() => { 
    requestData('POST', '/Request', JSON.stringify(DATA))
    .then(response => document.getElementById('text').textContent = x);
    x++;
}, interval);

//setInterval(postData('/Request', { DATA }), 1000);

document.getElementById("button").addEventListener("click", () => {
    document.getElementById('text').textContent = "click";
    requestData('POST', '/Request', JSON.stringify(DATA))
        .then(response => document.getElementById('text').textContent = JSON.stringify(response));
});

const requestData = async (method, url, data = null) => { //Функция отправляет POST запрос и получает ответ.
    const res = await fetch(url, {
        method: method,
        headers: {'Content-Type': 'application/json;charset=utf-8'},
        body: data
    })
    if (!res.ok) {
        console.log('!res.ok=true');
    }
    return await res.json();
}