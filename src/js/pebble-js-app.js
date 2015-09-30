// Page for app configuration
Pebble.addEventListener('showConfiguration', function(e) {
  var encodeQueryData = function (data)
  {
      return Object.keys(data).map(function(key) {
         return [key, data[key]].map(encodeURIComponent).join("=");
     }).join("&");
  };

  var baseUrl = 'http://tammyd.github.io/NHLScoreboard2/config/nhlscoreboard.html';
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
    var configuration = JSON.parse(decodeURIComponent(e.response));
    console.log('Configuration window returned: ' + JSON.stringify(configuration));

    var tempInF = configuration.tempUnitF ? true : false;

    setToLocalStorage(KEY_CONFIG_TEMP_UNIT_F, tempInF);

    // Send to Pebble
    Pebble.sendAppMessage({
      KEY_CONFIG_TEMP_UNIT_F : tempInF,
    },
      function(e) {
        console.log("Weather info sent to Pebble successfully!");
      },
      function(e) {
        console.log("Error sending weather info to Pebble!");
      }
    );

    getWeather();
  }
);

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
      pos.coords.latitude + "&lon=" + pos.coords.longitude;

  console.log(url);

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET',
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);

      // Temperature in Kelvin requires adjustment
      var temperature = Math.round(json.main.temp - 273.15);
      console.log("Temperature is " + temperature);

      // Conditions
      var conditions = json.weather[0].main;
      console.log("Conditions are " + conditions);

      // Assemble dictionary using our keys
      var dictionary = {
        "KEY_TEMPERATURE": temperature,
        "KEY_CONDITIONS": conditions
      };

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
