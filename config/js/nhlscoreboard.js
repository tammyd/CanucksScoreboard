
var config = function() {

  var getQueryParam = function(variable, defaultValue) {
    // Find all URL parameters
    var query = location.search.substring(1);
    var vars = query.split('&');
    for (var i = 0; i < vars.length; i++) {
      var pair = vars[i].split('=');

      // If the query variable parameter is found, decode it to use and return it for use
      if (pair[0] === variable) {
        return decodeURIComponent(pair[1]);
      }
    }
    return defaultValue || false;
  };


  var getBoolQueryParam = function(variable, defaultValue) {
    var value = getQueryParam(variable, defaultValue);
    var bool = (
      value == 1 ||
      value === true ||
      value == 'true' ||
      value == "True" ||
      value == "yes" ||
      value == "Yes"
    );
    return bool;
  };

  var init = function() {
    $("[id='tempSwitch']").bootstrapSwitch('state', getBoolQueryParam('tempUnitF', false));
  };

  var get = function() {
    var data = {
      tempUnitF: $("[id='tempSwitch']").bootstrapSwitch('state'),
    };
    console.log(JSON.stringify(data));
    return data;
  };

  var submit = function() {
    var data = get();
    var return_to = getQueryParam('return_to', 'pebblejs://close#');
    var location = return_to + encodeURIComponent(JSON.stringify(data));
    console.log("Returning to: " + location);
    window.location = location;
    return false;
  };

  return {
    init: init,
    get: get,
    submit: submit
  };
}();

config.init();
