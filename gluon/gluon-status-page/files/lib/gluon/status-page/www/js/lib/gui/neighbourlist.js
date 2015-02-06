"use strict";
define([ "lib/helper"
       , "lib/gui/signalgraph"
       ], function (Helper, SignalGraph) {

  function ListEntry(parent, id, stream, mgmtBus) {
    var el = document.createElement("li");
    el.id = id;

    var wrapper = document.createElement("div");
    wrapper.className = "wrapper";

    var canvas = document.createElement("canvas");
    el.appendChild(wrapper);
    wrapper.appendChild(canvas);
    parent.appendChild(el);

    canvas.className = "signal-history";
    canvas.height = 100;
    var chart = new SignalGraph(canvas, -100, 0, 5, true);

    var info = document.createElement("div");
    info.className = "info";

    var hostname = document.createElement("h3");
    info.appendChild(hostname);

    var batadv = document.createElement("p");
    info.appendChild(batadv);

    wrapper.appendChild(info);

    var infoSet = false;
    var unsubscribe = stream.onValue(update);

    el.destroy = function () {
      unsubscribe();
    }

    return el;

    function update(d) {
      if ("wifi" in d) {
        var signal = d.wifi.signal;
        var inactive = d.wifi.inactive;

        if (inactive > 200)
          signal = null;

        chart(signal);
      }

      if ("batadv" in d) {
        batadv.textContent = "TQ: " + d.batadv.tq + " (" + d.batadv.lastseen + "s)";
      } else {
        batadv.textContent = "";
      }

      if (!infoSet) {
        while (hostname.firstChild)
          hostname.removeChild(hostname.firstChild);

        if ("nodeInfo" in d) {
          infoSet = true;

          var link = document.createElement("a");
          link.textContent = d.nodeInfo.hostname;
          link.href = "#";
          link.nodeInfo = d.nodeInfo;
          link.onclick = function () {
            mgmtBus.pushEvent("goto", this.nodeInfo);
            return false;
          }

          hostname.appendChild(link);
          hostname.appendChild(document.createTextNode(" "));
        }

        hostname.appendChild(document.createTextNode("(" + d.id + ")"));
      }
    }
  }

  function Interface(iface, stream, mgmtBus) {
    var el = document.createElement("div");
    el.ifname = iface;

    var h = document.createElement("h3");
    h.textContent = iface;
    el.appendChild(h);

    var list = document.createElement("ul");
    list.className = "list-neighbour";
    el.appendChild(list);

    var stopStream = stream.skipDuplicates(sameKeys).onValue(update);

    function update(d) {
      var have = {};
      var remove = [];
      if (list.hasChildNodes()) {
        var children = list.childNodes;
        for (var i = 0; i < children.length; i++) {
          var a = children[i];
          if (a.id in d)
            have[a.id] = true;
          else {
            a.destroy();
            remove.push(a);
          }
        }
      }

      remove.forEach(function (d) { list.removeChild(d); });

      for (var k in d)
        if (!(k in have))
          new ListEntry(list, k, stream.map("." + k), mgmtBus);
    }


    el.destroy = function () {
      stopStream();

      while (list.firstChild) {
        list.firstChild.destroy();
        list.removeChild(list.firstChild);
      }

      el.removeChild(h);
      el.removeChild(list);
    }

    return el;
  }

  function sameKeys(a, b) {
    var a = Object.keys(a).sort();
    var b = Object.keys(b).sort();

    return !(a < b || a > b);
  }

  return function (stream, mgmtBus) {
    var el = document.createElement("div");

    var stopStream = stream.skipDuplicates(sameKeys).onValue(update);

    function update(d) {
      var have = {};
      var remove = [];
      if (el.hasChildNodes()) {
        var children = el.childNodes;
        for (var i = 0; i < children.length; i++) {
          var a = children[i];
          if (a.ifname in d)
            have[a.ifname] = true;
          else {
            a.destroy();
            remove.push(a);
          }
        }
      }

      remove.forEach(function (d) { el.removeChild(d); });

      for (var k in d)
        if (!(k in have))
          el.appendChild(new Interface(k, stream.map("." + k), mgmtBus));
    }

    function destroy() {
      stopStream();

      while (el.firstChild) {
        el.firstChild.destroy();
        el.removeChild(el.firstChild);
      }
    }

    return { title: document.createTextNode("Nachbarknoten")
           , content: el
           , destroy: destroy
           }
  }
})
