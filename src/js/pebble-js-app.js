// START ANALYTICS from https://github.com/r-dent/pebble-analytics
var Analytics=function(t,e,a){this.analyticsId=t,this.appName=e,this.appVersion=a,this.analyticsUserId=Math.round(1e7*Math.random())};Analytics.prototype._trackGA=function(t,e){var a=new XMLHttpRequest,n="http://www.google-analytics.com/collect",i="v=1&tid="+this.analyticsId+"&cid="+this.analyticsUserId+"&t="+t+"&an="+this.appName+"&av="+this.appVersion;for(parameterKey in e)e.hasOwnProperty(parameterKey)&&(i+="&"+parameterKey+"="+e[parameterKey]);a.open("POST",n,!0),a.setRequestHeader("Content-length",i.length),a.send(i)},Analytics.prototype.trackScreen=function(t){this._trackGA("appview",{cd:t})},Analytics.prototype.trackEvent=function(t,e){this._trackGA("event",{ec:t,ea:e})};
var ga = new Analytics('UA-46485905-3', 'CanucksScoreboard', '1.0.0');

var KEY_CONFIG_TEMP_UNIT_F = 'KEY_CONFIG_TEMP_UNIT_F';

var config = {};

// Page for app configuration
Pebble.addEventListener('showConfiguration', function(e) {
  var encodeQueryData = function (data)
  {
      return Object.keys(data).map(function(key) {
         return [key, data[key]].map(encodeURIComponent).join("=");
     }).join("&");
  };

  var baseUrl = 'http://tammyd.github.io/CanucksScoreboard/config/index.html';
  var params = {
    'tempUnitF' : getFromLocalStorage(KEY_CONFIG_TEMP_UNIT_F, false),
  };
  var url = baseUrl + "?" + encodeQueryData(params);
  console.log("Loading: " + url);
  ga.trackEvent('config', 'opened');
  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed',
  function(e) {
    if (decodeURIComponent(e.response)) {

      var configuration = JSON.parse(decodeURIComponent(e.response));
      console.log('Configuration window returned: ' + JSON.stringify(configuration));

      config['tempUnitF'] = configuration.tempUnitF ? true : false;

      setToLocalStorage(KEY_CONFIG_TEMP_UNIT_F, config['tempUnitF']);

      // Send to Pebble
      Pebble.sendAppMessage({
        KEY_CONFIG_TEMP_UNIT_F : config['tempUnitF'],
      },
        function(e) {
          console.log("Sucessfully sent config");
        },
        function(e) {
          console.log("Error sending config info!");
        }
      );
    }

    console.log("Config: " + JSON.stringify(config));

    ga.trackEvent('config', 'closed');
    ga.trackEvent('tempUnits', config['tempUnitF'] ? "F" : "C");
    getWeather();
  }
);

var getApiKey = function() {
  
  var keys = [
    'ae018a0ee8d729fce80c45a60e805b51', 
    'e13621344541dc8765b09008d033f769'
  ]
  
  return keys[Math.floor(Math.random() * keys.length)];
  
}

var getFromLocalStorage = function(key, defaultValue) {
  var value = localStorage.getItem(key);

  if (value=="True" || value=="true") {
    value = true;
  }
  if (value=="False" || value=="false") {
    value = false;
  }
  if (value === null) {
    value = defaultValue;
  }

  return value;
};

var setToLocalStorage = function(key, value) {
  localStorage.setItem(key, value);
};

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  // Construct URL
  var url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
      pos.coords.latitude + "&lon=" + pos.coords.longitude + "&APPID=" + getApiKey();

  console.log(url);

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET',
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      var temperature;

      // Temperature in Kelvin requires adjustment
      if (config['tempUnitF']) {
        console.log("Calculating temp in F");
        temperature = Math.round((json.main.temp - 273.15) * 1.8 + 32);
      } else {
        console.log("Calculating temp in C");
        temperature = Math.round(json.main.temp - 273.15);
      }

      console.log("Temperature is " + temperature + (config['tempUnitF'] ? "F" : "C"));

      // Conditions
      var conditions = json.weather[0].main;
      console.log("Conditions are " + conditions);

      // Assemble dictionary using our keys
      var dictionary = {
        "KEY_CONFIG_TEMP_UNIT_F" : config['tempUnitF'],
        "KEY_TEMPERATURE": temperature,
        "KEY_CONDITIONS": conditions
      };

      ga.trackEvent('weather', json.sys.country);
      ga.trackEvent('location', pos.coords.latitude + ";" + pos.coords.longitude);

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Weather info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
        }
      );
    }
  );
}

function locationError(err) {
  console.log("Error requesting location!");
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready',
  function(e) {
    console.log("PebbleKit JS ready!");

    config['tempUnitF'] = getFromLocalStorage(KEY_CONFIG_TEMP_UNIT_F, false);

    console.log("Config: " + JSON.stringify(config));

    // Get the initial weather
    getWeather();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    getWeather();
  }
);
