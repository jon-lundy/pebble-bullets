// with thanks to Chris, from
// https://ninedof.wordpress.com/2014/05/24/pebble-sdk-2-0-tutorial-9-app-configuration/

Pebble.addEventListener("showConfiguration",
  function(e) {
    //Load the remote config page
    Pebble.openURL("https://jon-lundy.github.io/bullet_settings.html");
  }
);

Pebble.addEventListener("webviewclosed",
  function(e) {
    //Get JSON dictionary
    var configuration = JSON.parse(decodeURIComponent(e.response));
    console.log("Configuration window returned: " + JSON.stringify(configuration));
 
    //Send to Pebble, persist there
    Pebble.sendAppMessage(
      {"KEY_12H_TIME": configuration.use12hTime,
       "KEY_HIDE_BATTERY": configuration.hideBattery,
       "KEY_HIDE_DATE": configuration.hideDate},
      function(e) {
        console.log("Sending settings data...");
      },
      function(e) {
        console.log("Settings feedback failed!");
      }
    );
  }
);