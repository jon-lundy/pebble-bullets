// with thanks to Chris, from
// https://ninedof.wordpress.com/2014/05/24/pebble-sdk-2-0-tutorial-9-app-configuration/

Pebble.addEventListener("showConfiguration",
  function(e) {
    // grab the last saved values, so we can send them to the page
    var use12hTime = localStorage.getItem("use12hTime");
    var hideBattery = localStorage.getItem("hideBattery");
    var hideDate = localStorage.getItem("hideDate");
    
    // these could be null if we haven't saved before!
    if (use12hTime === null) {
        use12hTime = 0;
    }
    if (hideBattery === null) {
        hideBattery = 0;
    }
    if (hideDate === null) {
        hideDate = 0;
    }
    
    var oldOptions = {
      use12hTime: use12hTime,
      hideBattery: hideBattery,
      hideDate: hideDate
    };
    
    // build a GET URI with the old values
    var oldOptionsStr = "";
    for (var key in oldOptions) {
        if (oldOptionsStr !== "") {
            oldOptionsStr += "&";
        }
        oldOptionsStr += encodeURIComponent(key) + "=" + encodeURIComponent(oldOptions[key]);
    }
    
    //Load the remote config page
    Pebble.openURL("https://jon-lundy.github.io/bullet_settings_1_1.html?" + oldOptionsStr);
  }
);

Pebble.addEventListener("webviewclosed",
  function(e) {
    //Get JSON dictionary
    var configuration = JSON.parse(decodeURIComponent(e.response));
    console.log("Configuration window returned: " + JSON.stringify(configuration));
    
    // store the updated values in the JS side, so we'll have them next time we launch settings
    localStorage.setItem("use12hTime", configuration.use12hTime);
    localStorage.setItem("hideBattery", configuration.hideBattery);
    localStorage.setItem("hideDate", configuration.hideDate);
    
    // send the updated values to the Pebble
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