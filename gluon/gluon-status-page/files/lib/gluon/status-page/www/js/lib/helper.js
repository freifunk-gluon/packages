"use strict";
define([ "vendor/bacon" ], function (Bacon) {
  function get(url) {
    return Bacon.fromBinder(function(sink) {
      var req = new XMLHttpRequest();
      req.open("GET", url);

      req.onload = function() {
        if (req.status == 200)
          sink(new Bacon.Next(req.response));
        else
          sink(new Bacon.Error(req.statusText));
        sink(new Bacon.End());
      };

      req.onerror = function() {
        sink(new Bacon.Error("network error"));
        sink(new Bacon.End());
      };

      req.send();

      return function () {};
    });
  }

  function getJSON(url) {
    return get(url).map(JSON.parse);
  }

  function buildUrl(ip, object, param) {
    var url = "http://[" + ip + "]/cgi-bin/" + object;
    if (param) url += "?" + param;

    return url;
  }

  function request(ip, object, param) {
    return getJSON(buildUrl(ip, object, param));
  }

  function dictGet(dict, key) {
    var k = key.shift();

    if (!(k in dict))
      return null;

    if (key.length == 0)
      return dict[k];

    return dictGet(dict[k], key);
  }

  return { buildUrl: buildUrl
         , request: request
         , getJSON: getJSON
         , dictGet: dictGet
         }
})
