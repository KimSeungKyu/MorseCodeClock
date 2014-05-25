var uri = "http://dl.dropboxusercontent.com/u/92292537/mcc-config.html";

Pebble.addEventListener("ready",
  function(e) {
    console.log("PebbleKit JS ready!");
  }
);

Pebble.addEventListener("showConfiguration",
  function(e) {
    //Load the remote config page
    var options = JSON.parse(localStorage.getItem('options'));
    console.log("read options: " + JSON.stringify(options));
    console.log("showing configuration");
    if (options !== null) {
        uri = uri + '?' + 'invert_select=' + encodeURIComponent(options.invert);
    }
    Pebble.openURL(uri);
    //Pebble.openURL("https://dl.dropboxusercontent.com/u/92292537/mcc-config.html");
  }
);

Pebble.addEventListener("webviewclosed",
  function(e) {
    //Get JSON dictionary
    var configuration = JSON.parse(decodeURIComponent(e.response));
    console.log("Configuration window returned: " + JSON.stringify(configuration));
    localStorage.setItem('options', JSON.stringify(configuration));
    
    //Send to Pebble, persist there
    Pebble.sendAppMessage(
      {"KEY_INVERT": configuration.invert},
      function(e) {
        console.log("Sending settings data...");
      },
      function(e) {
        console.log("Settings feedback failed!");
      }
    );
  }
);