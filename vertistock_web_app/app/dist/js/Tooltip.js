/******/ (() => { // webpackBootstrap
/******/ 	var __webpack_modules__ = ({

/***/ "./src/js/Tooltip.js":
/*!***************************!*\
  !*** ./src/js/Tooltip.js ***!
  \***************************/
/***/ (function(module, exports, __webpack_require__) {

var __WEBPACK_AMD_DEFINE_FACTORY__, __WEBPACK_AMD_DEFINE_RESULT__;/**
 * Tooltip.js
 * A basic script that applies a mouseover tooltip functionality to all elements of a page that have a data-tooltip attribute
 * Matthias Schuetz, https://github.com/matthias-schuetz
 *
 * Copyright (C) Matthias Schuetz
 * Free to use under the MIT license
 */

(function (root, factory) {
  if (true) {
    // AMD. Register as an anonymous module.
    !(__WEBPACK_AMD_DEFINE_FACTORY__ = (factory),
		__WEBPACK_AMD_DEFINE_RESULT__ = (typeof __WEBPACK_AMD_DEFINE_FACTORY__ === 'function' ?
		(__WEBPACK_AMD_DEFINE_FACTORY__.call(exports, __webpack_require__, exports, module)) :
		__WEBPACK_AMD_DEFINE_FACTORY__),
		__WEBPACK_AMD_DEFINE_RESULT__ !== undefined && (module.exports = __WEBPACK_AMD_DEFINE_RESULT__));
  } else {}
})(this, function () {
  var _options = {
    tooltipId: "tooltip",
    offsetDefault: 15
  };
  var _tooltips = [];
  var _tooltipsTemp = null;
  function _bindTooltips(resetTooltips) {
    if (resetTooltips) {
      _tooltipsTemp = _tooltips.concat();
      _tooltips = [];
    }
    Array.prototype.forEach.call(document.querySelectorAll("[data-tooltip]"), function (elm, idx) {
      var tooltipText = (elm.getAttribute("title") || "").trim();
      var options;
      if (resetTooltips && _tooltipsTemp.length && _tooltipsTemp[idx] && _tooltipsTemp[idx].text) {
        if (tooltipText.length === 0) {
          elm.setAttribute("title", _tooltipsTemp[idx].text);
          tooltipText = _tooltipsTemp[idx].text;
        }
        elm.removeEventListener("mousemove", _onElementMouseMove);
        elm.removeEventListener("mouseout", _onElementMouseOut);
        elm.removeEventListener("mouseover", _onElementMouseOver);
      }
      if (tooltipText) {
        elm.setAttribute("title", "");
        elm.setAttribute("data-tooltip-id", idx);
        options = _parseOptions(elm.getAttribute("data-tooltip"));
        _tooltips[idx] = {
          text: tooltipText,
          options: options
        };
        elm.addEventListener("mousemove", _onElementMouseMove);
        elm.addEventListener("mouseout", _onElementMouseOut);
        elm.addEventListener("mouseover", _onElementMouseOver);
      }
    });
    if (resetTooltips) {
      _tooltipsTemp = null;
    }
  }
  function _createTooltip(text, tooltipId) {
    var tooltipElm = document.createElement("div");
    var tooltipText = document.createTextNode(text);
    var options = tooltipId && _tooltips[tooltipId] && _tooltips[tooltipId].options;
    if (options && options["class"]) {
      tooltipElm.setAttribute("class", 'tooltip-js ' + options["class"]);
    } else {
      tooltipElm.setAttribute("class", 'tooltip-js');
    }
    tooltipElm.setAttribute("id", _options.tooltipId);
    tooltipElm.appendChild(tooltipText);
    document.querySelector("body").appendChild(tooltipElm);
  }
  function _getTooltipElm() {
    return document.querySelector("#" + _options.tooltipId);
  }
  function _onElementMouseMove(evt) {
    var tooltipId = this.getAttribute("data-tooltip-id");
    var tooltipElm = _getTooltipElm();
    var options = tooltipId && _tooltips[tooltipId] && _tooltips[tooltipId].options;
    var offset = options && options.offset || _options.offsetDefault;
    var scrollY = window.scrollY || window.pageYOffset;
    var scrollX = window.scrollX || window.pageXOffset;
    var tooltipTop = evt.pageY + offset;
    var tooltipLeft = evt.pageX + offset;
    if (tooltipElm) {
      tooltipTop = tooltipTop - scrollY + tooltipElm.offsetHeight + 20 >= window.innerHeight ? tooltipTop - tooltipElm.offsetHeight - 20 : tooltipTop;
      tooltipLeft = tooltipLeft - scrollX + tooltipElm.offsetWidth + 20 >= window.innerWidth ? tooltipLeft - tooltipElm.offsetWidth - 20 : tooltipLeft;
      tooltipElm.style.top = tooltipTop + "px";
      tooltipElm.style.left = tooltipLeft + "px";
    }
  }
  function _onElementMouseOut(evt) {
    var tooltipElm = _getTooltipElm();
    if (tooltipElm) {
      document.querySelector("body").removeChild(tooltipElm);
    }
  }
  function _onElementMouseOver(evt) {
    var tooltipId = this.getAttribute("data-tooltip-id");
    var tooltipText = tooltipId && _tooltips[tooltipId] && _tooltips[tooltipId].text;
    if (tooltipText) {
      _createTooltip(tooltipText, tooltipId);
    }
  }
  function _parseOptions(options) {
    var optionsObj;
    if (options.length) {
      try {
        optionsObj = JSON.parse(options.replace(/'/ig, "\""));
      } catch (err) {
        console.log(err);
      }
    }
    return optionsObj;
  }
  function _init() {
    window.addEventListener("load", _bindTooltips);
  }
  _init();
  return {
    setOptions: function setOptions(options) {
      for (var option in options) {
        if (_options.hasOwnProperty(option)) {
          _options[option] = options[option];
        }
      }
    },
    refresh: function refresh() {
      _bindTooltips(true);
    }
  };
});

/***/ })

/******/ 	});
/************************************************************************/
/******/ 	// The module cache
/******/ 	var __webpack_module_cache__ = {};
/******/ 	
/******/ 	// The require function
/******/ 	function __webpack_require__(moduleId) {
/******/ 		// Check if module is in cache
/******/ 		var cachedModule = __webpack_module_cache__[moduleId];
/******/ 		if (cachedModule !== undefined) {
/******/ 			return cachedModule.exports;
/******/ 		}
/******/ 		// Create a new module (and put it into the cache)
/******/ 		var module = __webpack_module_cache__[moduleId] = {
/******/ 			// no module.id needed
/******/ 			// no module.loaded needed
/******/ 			exports: {}
/******/ 		};
/******/ 	
/******/ 		// Execute the module function
/******/ 		__webpack_modules__[moduleId].call(module.exports, module, module.exports, __webpack_require__);
/******/ 	
/******/ 		// Return the exports of the module
/******/ 		return module.exports;
/******/ 	}
/******/ 	
/************************************************************************/
/******/ 	
/******/ 	// startup
/******/ 	// Load entry module and return exports
/******/ 	// This entry module is referenced by other modules so it can't be inlined
/******/ 	var __webpack_exports__ = __webpack_require__("./src/js/Tooltip.js");
/******/ 	
/******/ })()
;