let id;
let target;
let options;
let GPS = document.getElementById("GPS");
GPS.style.display = "none";
let altitude = document.getElementById("altitude");
let altitudeVal = document.getElementById("altitudeVal");
altitude.style.display = "none";

var strGET = window.location.search.replace('?', '');//определяем get запрос
if (strGET === 'GPS=true') {
  useGPS = true;
  arrow.style.transitionDuration = "2s";
}

function success(position) {
  let speed = position.coords.speed * 3.6;
  if (useGPS === false)
    if (speed !== null && speed > 3) {
      GPS.style.display = "block";
      GPS.setAttribute('transform', `translate(0,0) rotate(${speed * 6} ` + x + " " + y + ")"); //7370" cy="5539//4475,8418 6161,6595 7847,4772 7984,4908 8121,5045 6298,6731
    } else {
      GPS.style.display = "none";
    }

  if (useGPS === true) {
    DATA.Velocity.Current_speed=speed;
    DATA.Odometer.daily-= DATA.Velocity.Current_speed/3600;
console.log(DATA.Odometer.daily);
    //arrow.setAttribute('transform', `translate(0,0) rotate(${speed * 6} ` + x + " " + y + ")"); //7370" cy="5539//4475,8418 6161,6595 7847,4772 7984,4908 8121,5045 6298,6731
    
  }



  if (position.coords.altitude !== null) {
    altitude.style.display = "block";
    altitudeVal.textContent = position.coords.altitude.toFixed(0) + " m"
  } else {
    altitude.style.display = "none";
  }

}

function error(err) {
  console.error(`ERROR(${err.code}): ${err.message}`);
}

options = {
  enableHighAccuracy: true,
  timeout: 1000,
  maximumAge: 0,
};

id = navigator.geolocation.watchPosition(success, error, options);

//пенсионный фонд: что за регистрация нужна пенсионерам?
//маме перерегистрируют удостоверения чтобы выяснить кого не стало