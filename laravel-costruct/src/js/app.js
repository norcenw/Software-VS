/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/**********************************library***************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
import lottie from 'lottie-web';

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/**************************global variable***************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

const ip = "vertistock.local";
const errors = [];
const hideTime = 5000;
var isValid = true;

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/*****************************eo-translator**************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

const savedLang = localStorage.getItem("language") || "en";
document.getElementById("languageSwitcher").value = savedLang;
localStorage.setItem("language", savedLang);
translatePage();

function translatePage() {
  const lang = localStorage.getItem("language") || "en";
  if (!translations[lang]) return;

  document.querySelectorAll("[eo-translator]").forEach(el => {
    const key = el.getAttribute("eo-translator");
    const newText = translations[lang][key] || key;
    el.innerText = newText;

    const parentBtn = el.closest("[data-tooltip]");
    if (parentBtn) {
      parentBtn.removeAttribute("title");
      parentBtn.setAttribute("data-tooltip", newText);
    }
  });
}

document.getElementById("languageSwitcher").addEventListener("change", function () {
  localStorage.setItem("language", this.value);
  hideAllVisibleTooltips();
  translatePage();
});

window.addEventListener("load", async () => {
  translatePage();
  getWarehouse();
  status()
  setInterval(() => {
    status()
  }, 2500);
});

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/*****************************data-tooltip***************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
let tooltipTimeout;

document.querySelectorAll('.data-tooltip-container').forEach(container => {
  container.addEventListener('click', (event) => {

    const existingTooltip = document.querySelector('.custom-tooltip.show');
    if (existingTooltip) {
      existingTooltip.remove();
      clearTimeout(tooltipTimeout);
    }

    const tooltipText = container.getAttribute('data-tooltip') || '';
    if (!tooltipText) return;

    const tooltipEl = document.createElement('div');
    tooltipEl.classList.add('custom-tooltip');
    tooltipEl.innerHTML = `<div>${tooltipText}</div>`;

    document.body.appendChild(tooltipEl);

    const rect = container.getBoundingClientRect();
    const tooltipRect = tooltipEl.getBoundingClientRect();
    const screenMid = window.innerWidth / 2;
    const containerCenterX = rect.left + (rect.width / 2);

    let left, top;
    if (containerCenterX < screenMid) {
      tooltipEl.classList.add('right-position');
      left = rect.right + 10;
      top = rect.top + (rect.height / 2) - (tooltipRect.height / 2);
    } else {
      tooltipEl.classList.add('left-position');
      left = rect.left - tooltipRect.width - 10;
      top = rect.top + (rect.height / 2) - (tooltipRect.height / 2);
    }

    tooltipEl.style.left = left + 'px';
    tooltipEl.style.top = top + 'px';

    requestAnimationFrame(() => {
      tooltipEl.classList.add('show');
    });

    tooltipTimeout = setTimeout(() => {
      tooltipEl.remove();
    }, 4000);

  });
});

function hideAllVisibleTooltips() {
  const visibleTooltips = document.querySelectorAll('.custom-tooltip.show');
  if (!visibleTooltips.length) return;

  visibleTooltips.forEach(tooltip => {
    tooltip.remove();
  });

  clearTimeout(tooltipTimeout);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/*********************************animation**************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

function showLoadingAnimation(animation, text) {
  document.querySelector("#layover").style.display = "block";
  document.querySelector("#text-lottie").style.display = "block";
  switch (animation) {
    case "loading":
      document.querySelector("#animation-loading").style.display = "block";
      break;
    case "correct":
      document.querySelector("#animation-state").style.display = "block";
      break;
    default:
      document.querySelector("#animation-state").style.display = "block";
      break;
  }

  let newText = text.replaceAll(",", "");
  let textLottie = document.querySelector("#text-lottie").innerHTML = newText;
  document.querySelector("#text-lottie").setAttribute("eo-translator", newText);
  document.querySelector("#text-lottie").setAttribute("eo-translator-html", "true");
  translatePage();

  lottie.loadAnimation({
    container:
      animation == "loading"
        ? document.querySelector("#animation-loading")
        : document.querySelector("#animation-state"),
    renderer: "svg",
    loop: true,
    autoplay: true,
    path: `../dist/resources/lottie/${animation}.json`,
  });
}

function hideLoadingAnimation() {
  document.querySelector("#layover").innerHTML = "";
  document.querySelector("#animation-loading").innerHTML = "";
  document.querySelector("#animation-state").innerHTML = "";
  document.querySelector("#text-lottie").innerHTML = "";
  document.querySelector("#layover").style.display = "none";
  document.querySelector("#animation-loading").style.display = "none";
  document.querySelector("#animation-state").style.display = "none";
  document.querySelector("#text-lottie").style.display = "none";
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************config log**************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

function getDate() {
  const now = new Date();

  const year = now.getFullYear();
  const month = String(now.getMonth() + 1).padStart(2, "0");
  const day = String(now.getDate()).padStart(2, "0");

  const hours = String(now.getHours()).padStart(2, "0");
  const minutes = String(now.getMinutes()).padStart(2, "0");
  const seconds = String(now.getSeconds()).padStart(2, "0");

  return `<span eo-translator="date" eo-translator-true>Date</span>:${day}/${month}/${year}_${hours}:${minutes}:${seconds};<br>`;
}

function getLocalStorageSize() {
  return Object.keys(localStorage).reduce((total, key) => {
    let item = localStorage.getItem(key);
    return total + (item ? item.length : 0);
  }, 0);
}

function checkStorageSize() {
  if (getLocalStorageSize() > 5000) {
    console.warn("Limite superato! Svuotamento archiviazione locale...");
    localStorage.removeItem("Error");
    localStorage.removeItem("Log");
  }
}

function setError(error) {
  localStorage.setItem("Error", error);
  checkStorageSize();
}

function setLog(log) {
  localStorage.setItem("Log", log);
  checkStorageSize();
}

function getLog() {
  return localStorage.getItem("Log") || "";
}

function getError() {
  return localStorage.getItem("Error") || "";
}

function colorizeLog() {
  let logContainer = document.getElementById("log");
  let logText = logContainer.innerHTML;

  let patternV = /V.*?V/g;
  logText = logText.replace(patternV, function (match) {
    return `<span style="color: #28a745;">${match}</span>`;
  });

  let patternX = /X.*?X/g;
  logText = logText.replace(patternX, function (match) {
    if (
      match.includes("torqueX") ||
      match.includes("voltageX") ||
      match.includes("wattX") ||
      match.includes("wattX2")
    ) {
      return match;
    }
    return `<span style="color: #fe0808;">${match}</span>`;
  });

  let patternExclamation = /!.*?!/g;
  logText = logText.replace(patternExclamation, function (match) {
    return `<span style="color: #ffd214;">${match}</span>`;
  });

  let patternDash = /-----.*?-----/g;
  logText = logText.replace(patternDash, function (match) {
    return `<span style="color: #076cd7;" eo-translator="${match}" eo-translator-html="true">${match}</span>`;
  });

  logContainer.innerHTML = logText;
}

function addLog(text) {
  let currentLog = getLog();
  let newLog =
    currentLog +
    getDate() +
    "\n" +
    text.replaceAll(",", "") +
    "\n\n" +
    "<br><br>";
  setLog(newLog);
  let logCont = document.querySelector("#log");
  logCont.innerHTML = newLog;
  logCont.scrollTop = logCont.scrollHeight;
  colorizeLog();
  translatePage();
}

function addError(text) {
  let currentError = getError();
  let newError =
    currentError +
    getDate() +
    "\n" +
    text.replaceAll(",", "") +
    "\n\n" +
    "<br><br>";
  setError(newError);
  console.log(getError());
  translatePage();
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/**************************************get***************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

async function status() {
  let data = null;
  try {
    const response = fetch(`http://${ip}/op=status`, { method: "GET" });
    const result = await response;
    if (!result.ok) {
      const error = `XFatal Error: ${result.status}, Incorrect server response, get/op=statusX<br>`;
      errors.push(error);
      addLog(error);
      throw new Error(`Error: ${result.status}`);
    }

    data = await result.json();
    console.log(data);
    renderStatus(data);
  } catch (error) {
    checkArduinoStatus();
    errors.push(`X${error}X`);
    addLog(errors.toString());
    addError(errors.toString());
  }

  return data;
}

function checkArduinoStatus() {
  let arduinoConnect = document.querySelector("#arduino-connect");
  let low = document.querySelector("#low-stream");
  let mid = document.querySelector("#mid-stream");
  let high = document.querySelector("#high-stream");
  let no = document.querySelector("#no-stream");
  let rssi = document.querySelector("#rssi");

  if (arduinoConnect.classList.contains("connected")) {
    low.style.display = "none";
    mid.style.display = "none";
    high.style.display = "none";
    no.style.display = "flex";
    arduinoConnect.classList.remove("connected");
    addLog("!arduino is not connected!");
    rssi.innerHTML = "No Stream";
  }
}

async function getOp1() {
  addLog("-----loading arduino view-----");
  showLoadingAnimation("loading", "We are loading your arduino...");
  return new Promise((resolve, reject) => {
    fetch(`http://${ip}/op=r_1`, { method: "GET" })
      .then((response) => {
        if (!response.ok) {
          const error = `XFatal Error: ${response.status}, Incorrect server response, get/op=r_1X<br>`;
          errors.push(error);
          addLog(error);
          reject(new Error(`Error: ${response.status}`));
        } else {
          return response.json();
        }
      })
      .then((data) => {
        console.log(data);
        renderOp1(data);
        resolve(data);
        hideLoadingAnimation();
      })
      .catch((error) => {
        hideLoadingAnimation();
        errors.push(`X${error}X`);
        showLoadingAnimation("fail", errors.toString());
        addLog(`${errors.toString()}`);
        addError(`${errors.toString()}`);
        setTimeout(() => {
          hideLoadingAnimation();
          showLoadingAnimation("warning", "!Please reload the page!");
          addLog("!error to load!");
        }, hideTime);
      });
  });
}

async function toggleOp1() {
  addLog("-----loading arduino view-----");
  return new Promise((resolve, reject) => {
    fetch(`http://${ip}/op=r_1`, { method: "GET" })
      .then((response) => {
        if (!response.ok) {
          const error = `XFatal Error: ${response.status}, Incorrect server response, get/op=r_1X<br>`;
          errors.push(error);
          addLog(error);
          reject(new Error(`Error: ${response.status}`));
        } else {
          return response.json();
        }
      })
      .then((data) => {
        console.log(data);
        renderOp1(data);
        resolve(data);
      })
      .catch((error) => {
        errors.push(`X${error}X`);
        addLog(`${errors.toString()}`);
        addError(`${errors.toString()}`);
      });
  });
}

async function getOp2() {
  addLog("-----loading arduino ctrl-----");
  showLoadingAnimation("loading", "We are loading your arduino...");
  return new Promise((resolve, reject) => {
    fetch(`http://${ip}/op=r_2`, { method: "GET" })
      .then((response) => {
        if (!response.ok) {
          const error = `XFatal Error: ${response.status}, Incorrect server response, get/op=r_1X<br>`;
          errors.push(error);
          addLog(error);
          reject(new Error(`Error: ${response.status}`));
        } else {
          return response.json();
        }
      })
      .then((data) => {
        console.log(data);
        renderOp2(data);
        resolve(data);
        hideLoadingAnimation();
      })
      .catch((error) => {
        hideLoadingAnimation();
        errors.push(`X${error}X`);
        showLoadingAnimation("fail", errors.toString());
        addLog(`${errors.toString()}`);
        addError(`${errors.toString()}`);
        setTimeout(() => {
          hideLoadingAnimation();
          showLoadingAnimation("warning", "!Please reload the page!");
          addLog("!error to load!");
        }, hideTime);
      });
  });
}

async function getOp3wh1() {
  addLog("-----loading management 1-----");
  return new Promise((resolve, reject) => {
    fetch(`http://${ip}/op=r_3_1`, { method: "GET" })
      .then((response) => {
        if (!response.ok) {
          const error = `XFatal Error: ${response.status}, Incorrect server response, get/op=r_3X<br>`;
          errors.push(error);
          addLog(error);
          reject(new Error(`Error: ${response.status}`));
        } else {
          return response.json();
        }
      })
      .then((data) => {
        console.log(data);
        renderOp3(data);
        resolve(data);
      })
      .catch((error) => {
        hideLoadingAnimation();
        errors.push(`X${error}X`);
        showLoadingAnimation("fail", errors.toString());
        addLog(`${errors.toString()}`);
        addError(`${errors.toString()}`);
        setTimeout(() => {
          hideLoadingAnimation();
          showLoadingAnimation("warning", "!Please reload the page!");
          addLog("!error to load!");
        }, hideTime);
      });
  });
}

async function getOp3wh2() {
  addLog("-----loading management 2-----");
  return new Promise((resolve, reject) => {
    fetch(`http://${ip}/op=r_3_2`, { method: "GET" })
      .then((response) => {
        if (!response.ok) {
          const error = `XFatal Error: ${response.status}, Incorrect server response, get/op=r_3X<br>`;
          errors.push(error);
          addLog(error);
          reject(new Error(`Error: ${response.status}`));
        } else {
          return response.json();
        }
      })
      .then((data) => {
        console.log(data);
        renderOp3(data);
        resolve(data);
      })
      .catch((error) => {
        hideLoadingAnimation();
        errors.push(`X${error}X`);
        showLoadingAnimation("fail", errors.toString());
        addLog(`${errors.toString()}`);
        addError(`${errors.toString()}`);
        setTimeout(() => {
          hideLoadingAnimation();
          showLoadingAnimation("warning", "!Please reload the page!");
          addLog("!error to load!");
        }, hideTime);
      });
  });
}

async function getWarehouse() {
  let removeBox = document.querySelectorAll(".cube");
  removeBox.forEach((el) => {
    if (el.classList.contains("full-box")) {
      el.classList.remove("full-box");
    }
  });
  document.querySelector("#found").innerHTML = "";
  showLoadingAnimation("loading", "We are loading your Warehouse...");
  await getOp3wh1();
  await getOp3wh2();
  hideLoadingAnimation();
  translatePage();
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/***********************************render***************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

const renderOp2Array = [
  "xacc-write",
  "yacc-write",
  "zacc-write",
  "xdelay-write",
  "ydelay-write",
  "zdelay-write",
];

const renderOp1Array = [
  "xacc-read",
  "yacc-read",
  "zacc-read",
  "xdelay-read",
  "ydelay-read",
  "zdelay-read",
  "xspeed-read",
  "yspeed-read",
  "zspeed-read",
  "xtime-read",
  "ytime-read",
  "ztime-read",
  "xcoordinate-read",
  "ycoordinate-read",
  "zcoordinate-read",
  "xrun-read",
  "yrun-read",
  "zrun-read",
  "xstep-read",
  "ystep-read",
  "zstep-read"
];

function renderStatus(render) {
  let low = document.querySelector("#low-stream");
  let mid = document.querySelector("#mid-stream");
  let high = document.querySelector("#high-stream");
  let no = document.querySelector("#no-stream");
  let rssi = document.querySelector("#rssi");

  try {
    for (const key in render) {
      if (key === "status") {
        let arduinoConnect = document.querySelector("#arduino-connect");
        if (render[key] === "true" && !arduinoConnect.classList.contains("connected")) {
          arduinoConnect.classList.add("connected");
        }
      } else if (key === "rssi") {
        const rssiValue = Number(render[key]);
        rssi.innerHTML = render[key] + "rssi";

        low.style.display = "none";
        mid.style.display = "none";
        high.style.display = "none";
        no.style.display = "none";

        if (rssiValue > -30) {
          high.style.display = "block";
        }
        else if (rssiValue > -67) {
          mid.style.display = "block";
        }
        else if (rssiValue > -80) {
          low.style.display = "block";
        }
        else if (rssiValue > -90) {
          no.style.display = "block";
        }
        else {
          no.style.display = "block";
        }
      }
      else {
        console.log(`${key} non gestita`);
      }
    }
  } catch (error) {
    errors.push(`x${error}x`);
    addLog(`${errors.toString()}`);
    addError(`${errors.toString()}`);
    addLog("!error to load!");
  }
}

function renderOp1(render) {
  try {
    let jsonString = JSON.stringify(render);
    addLog(`VgetOp2: ${jsonString}V`);
    for (const key in render) {
      if (key == "xacc-read" || key == "yacc-read" || key == "zacc-read") {
        let selectSelect = document.querySelector(`select[name="${key}"]`);
        if (selectSelect) {
          selectSelect.value = render[key];
        }
      } else if (renderOp1Array.includes(key)) {
        let input = document.querySelector(`input[name="${key}"]`);
        if (input) {
          if (key == "xdelay-read" || key == "ydelay-read" || key == "zdelay-read") {
            input.value = render[key].toFixed(2) + "ms";
          } else if (key == "xspeed-read" || key == "yspeed-read" || key == "zspeed-read") {
            input.value = render[key] + "rpm";
          } else if (key == "xtime-read" || key == "ytime-read" || key == "ztime-read") {
            input.value = render[key] + "ms";
          } else if (key == "xstep-read" || key == "ystep-read" || key == "zstep-read" || key == "xcoordinate-read" || key == "ycoordinate-read" || key == "zcoordinate-read") {
            input.value = render[key] + "mm";
          } else if (key == "xrun-read" || key == "yrun-read" || key == "zrun-read") {
            if (render[key] == 0) {
              input.value = "⛔";
            } else if (render[key] == 1) {
              input.value = "⌛";
            } else if (render[key] == 2) {
              input.value = "✅";
            }
          } else {
            input.value = render[key];
          }
        }
      } else {
        console.log(`${key} non gestita`);
      }
    }
  } catch (error) {
    hideLoadingAnimation();
    errors.push(`x${error}x`);
    showLoadingAnimation("fail", errors.toString());
    addLog(`${errors.toString()}`);
    addError(`${errors.toString()}`);
    setTimeout(() => {
      hideLoadingAnimation();
      showLoadingAnimation("warning", "!Please reload the page!");
      addLog("!error to load!");
    }, hideTime);
  }
}

function renderOp2(render) {
  try {
    let jsonString = JSON.stringify(render);
    addLog(`VgetOp2: ${jsonString}V`);
    for (const key in render) {
      if (key == "xacc-write" || key == "yacc-write" || key == "zacc-write") {
        let selectSelect = document.querySelector(`select[name="${key}"]`);
        if (selectSelect) {
          selectSelect.value = render[key];
        }
      } else if (renderOp2Array.includes(key)) {
        let input = document.querySelector(`input[name="${key}"]`);
        if (input) {
          input.value = render[key];
          console.log(render[key]);
        }
      } else if (key == "wifi") {
        document.querySelector("#wifi-list").innerHTML = "";
        for (let i = 0; i < render[key].length; i++) {
          document.querySelector("#count-wifi").innerHTML = String(render[key].length);
          addWifiItem(i + 1, render[key][i].ssid, render[key][i].pass);
        }
      } else {
        console.log(`${key} non gestita`);
      }
    }
  } catch (error) {
    hideLoadingAnimation();
    errors.push(`x${error}x`);
    showLoadingAnimation("fail", errors.toString());
    addLog(`${errors.toString()}`);
    addError(`${errors.toString()}`);
    setTimeout(() => {
      hideLoadingAnimation();
      showLoadingAnimation("warning", "!Please reload the page!");
      addLog("!error to load!");
    }, hideTime);
  }
}

function addWifiItem(index, ssid, pass) {
  const container = document.querySelector("#wifi-list");
  const wifiItem = createWifiItem(index, ssid, pass);
  container.appendChild(wifiItem);
}

function createWifiItem(index, ssid, pass) {
  console.log(ssid);
  console.log(pass);
  const wifiItem = document.createElement("div");
  wifiItem.className = "wifi-item";
  wifiItem.draggable = true;

  const numberElement = document.createElement("p");
  numberElement.textContent = `${index}.`;
  wifiItem.appendChild(numberElement);

  const ssidContainer = document.createElement("div");
  const ssidInput = document.createElement("input");
  ssidInput.type = "text";
  ssidInput.name = `ssid-${index}`;
  ssidInput.placeholder = "ssid";
  ssidInput.className = "no-drag";
  ssidInput.value = ssid;
  ssidContainer.appendChild(ssidInput);
  wifiItem.appendChild(ssidContainer);

  const passwordContainer = document.createElement("div");
  const passwordInput = document.createElement("input");
  passwordInput.type = "password";
  passwordInput.name = `pass-${index}`;
  passwordInput.placeholder = "password";
  passwordInput.value = pass;
  passwordInput.className = "no-drag";

  const eyeImg = document.createElement("img");
  eyeImg.src = "resources/img/eye.svg";
  eyeImg.alt = "eye.svg";
  eyeImg.id = "show-password";

  const closeEyeImg = document.createElement("img");
  closeEyeImg.src = "resources/img/close-eye.svg";
  closeEyeImg.alt = "close-eye.svg";
  closeEyeImg.id = "hide-password";

  passwordContainer.appendChild(passwordInput);
  passwordContainer.appendChild(eyeImg);
  passwordContainer.appendChild(closeEyeImg);
  wifiItem.appendChild(passwordContainer);

  const wifiModContainer = document.createElement("div");
  wifiModContainer.className = "wifi-mod";

  const sortImg = document.createElement("img");
  sortImg.src = "resources/img/sort.svg";
  sortImg.alt = "sort.svg";

  const modContainer = document.createElement("div");
  modContainer.style.display = "flex";
  modContainer.style.alignItems = "center";
  modContainer.style.justifyContent = "space-between";

  const modifyImg = document.createElement("img");
  modifyImg.src = "resources/img/modify.svg";
  modifyImg.alt = "modify.svg";
  modifyImg.style.display = "flex"; // Mostrato di default

  const nomodifyImg = document.createElement("img");
  nomodifyImg.src = "resources/img/no-modify.svg";
  nomodifyImg.alt = "no-modify.svg";
  nomodifyImg.style.display = "none"; // Nascosto di default
  nomodifyImg.style.marginRight = "10px";

  const trashImg = document.createElement("img");
  trashImg.src = "resources/img/trash.svg";
  trashImg.alt = "trash.svg";
  trashImg.id = "remove-wifi";

  modContainer.appendChild(modifyImg);
  modContainer.appendChild(nomodifyImg);
  modContainer.appendChild(trashImg);

  wifiModContainer.appendChild(sortImg);
  wifiModContainer.appendChild(modContainer);
  wifiItem.appendChild(wifiModContainer);

  // Eventi per mostrare/nascondere password
  eyeImg.addEventListener("click", () => {
    passwordInput.type = "text";
    eyeImg.style.display = "none";
    closeEyeImg.style.display = "inline";
  });

  closeEyeImg.addEventListener("click", () => {
    passwordInput.type = "password";
    closeEyeImg.style.display = "none";
    eyeImg.style.display = "inline";
  });

  // Evento per attivare modifica
  modifyImg.addEventListener("click", () => {
    ssidInput.classList.remove("no-drag");
    passwordInput.classList.remove("no-drag");
    ssidInput.removeAttribute("readonly");
    passwordInput.removeAttribute("readonly");
    nomodifyImg.style.display = "flex";
    modifyImg.style.display = "none";
  });

  // Evento per disattivare modifica (opposto del precedente)
  nomodifyImg.addEventListener("click", () => {
    ssidInput.classList.add("no-drag");
    passwordInput.classList.add("no-drag");
    ssidInput.setAttribute("readonly", "true");
    passwordInput.setAttribute("readonly", "true");
    modifyImg.style.display = "flex";
    nomodifyImg.style.display = "none";
  });

  // Evento per eliminare l'elemento
  trashImg.addEventListener("click", () => {
    let countWifi = document.querySelector("#count-wifi");
    let numCount = parseInt(countWifi.innerHTML);
    countWifi.innerHTML = numCount - 1;
    wifiItem.remove();
  });

  return wifiItem;
}

function renderOp3(render) {
  try {
    let jsonString = JSON.stringify(render);
    console.log(jsonString);
    addLog(`VgetOp3whx: ${jsonString}V`);

    for (const key in render) {
      if (key == "wh1" || key == "wh2") {
        for (let i = 0; i < render[key].length; i++) {
          console.table(render[key].length);
          if (render[key][i].full == "1") {
            let x = render[key][i].x;
            let y = render[key][i].y;
            let z = render[key][i].z;
            console.log(`Looking for: .x-${x}.y-${y}.z-${z}`);
            let boxWarehouse = document.querySelector(`.x-${x}.y-${y}.z-${z}`);
            console.log("Found:", boxWarehouse);
            boxWarehouse.classList.add("full-box");

            let containerFound = document.querySelector("#found");
            let containerElement = document.createElement("div");
            containerElement.classList.add("container-element");
            containerElement.classList.add(`x-${x}`);
            containerElement.classList.add(`y-${y}`);
            containerElement.classList.add(`z-${z}`);
            let nameContainer = document.createElement("span");
            let nameText = document.createElement("p");
            nameText.textContent = "NAME:";
            nameText.setAttribute("eo-translator", "name");
            nameText.setAttribute("eo-translator-html", "true");
            let nameSpan = document.createElement("span");
            nameSpan.classList.add("name-el");
            nameSpan.textContent = render[key][i].name;
            nameContainer.appendChild(nameText);
            nameContainer.appendChild(nameSpan);

            let img = document.createElement("img");
            img.src = "../dist/resources/img/arrow.png";
            img.alt = "arrowUP";
            img.classList.add("info");

            let banner = document.createElement("div");
            banner.classList.add("banner");
            let codeP = document.createElement("p");
            codeP.textContent = "CODE:";
            codeP.setAttribute("eo-translator", "code");
            codeP.setAttribute("eo-translator-html", "true");
            let codeSpan = document.createElement("span");
            codeSpan.classList.add("code-el");
            codeSpan.textContent = render[key][i].code;

            let countP = document.createElement("p");
            countP.textContent = "COUNT:";
            countP.setAttribute("eo-translator", "count");
            countP.setAttribute("eo-translator-html", "true");
            let countSpan = document.createElement("span");
            countSpan.classList.add("number-el");
            countSpan.textContent = render[key][i].count;

            let buttonContainer = document.createElement("div");
            buttonContainer.classList.add("button-container-banner");
            let button = document.createElement("button");
            button.textContent = "WITHDRAW:";
            button.setAttribute("eo-translator", "withdraw");
            button.setAttribute("eo-translator-html", "true");
            buttonContainer.appendChild(button);
            button.classList.add(`x-${x}`);
            button.classList.add(`y-${y}`);
            button.classList.add(`z-${z}`);

            banner.appendChild(codeP);
            banner.appendChild(codeSpan);
            banner.appendChild(countP);
            banner.appendChild(countSpan);
            banner.appendChild(buttonContainer);
            containerElement.appendChild(nameContainer);
            containerElement.appendChild(img);
            containerElement.appendChild(banner);
            containerFound.appendChild(containerElement);

            button.addEventListener("click", function postWarehouse() {
              const result = {
                "box": i,
                "z": z,
              };
              let jsonString = JSON.stringify(result);
              console.log(jsonString);

              showLoadingAnimation("loading", "Withdraw...")
              const xhr = new XMLHttpRequest();
              xhr.open("POST", "http://" + ip + "/op=w_3", true);
              xhr.setRequestHeader("Content-Type", "application/json");
              xhr.onload = function () {
                if (xhr.status >= 200 && xhr.status < 300) {
                  setTimeout(() => {
                    hideLoadingAnimation();
                  }, 2000);
                  setTimeout(() => {
                    showLoadingAnimation("correct", "Success");
                  }, 2100);
                  setTimeout(() => {
                    hideLoadingAnimation();
                  }, 4100);
                } else {
                  addLog(`XError /op=withdraw: ${xhr.status}X`);
                  setTimeout(() => {
                    hideLoadingAnimation();
                  }, 2000);
                  setTimeout(() => {
                    showLoadingAnimation(
                      "warning",
                      `${xhr.status}`
                    );
                  }, 2100);
                }
              };

              xhr.onerror = function () {
                addLog("XError of Connection /op=X");
                showLoadingAnimation("fail", `Error`);
                setTimeout(() => {
                  hideLoadingAnimation();
                }, 2000);
              };
              xhr.send(jsonString);
              addLog(`Vwithdraw: ${jsonString}V`);

              xhr.onloadend = function () {
                setTimeout(() => {
                  getWarehouse();
                }, 4100);
              };
            });

          }
        }
      }
    }
    setTimeout(() => {
      initializeSearch();
    }, 0);
  } catch (error) {
    hideLoadingAnimation();
    errors.push(`x${error}x`);
    showLoadingAnimation("fail", errors.toString());
    addLog(`${errors.toString()}`);
    addError(`${errors.toString()}`);
    setTimeout(() => {
      hideLoadingAnimation();
      showLoadingAnimation("warning", "!Please reload the page!");
      addLog("!error to load!");
    }, hideTime);
  }
}

function initializeSearch() {
  var elements = document.querySelectorAll(".container-element");
  var searchName = document.querySelector("input[name=name-search]");
  var btnSearch = document.querySelector("#search");
  var reset = document.querySelector("#reset-search");
  var foundString = document.querySelector("#not-found");

  btnSearch.addEventListener("click", function () {
    let foundMatch = false;

    if (searchName.value !== "") {
      reset.classList.remove("no-reset");
      elements.forEach((el) => {
        let nameMatches = el.innerText
          .toLowerCase()
          .includes(searchName.value.toLowerCase());
        if (nameMatches) {
          foundMatch = true;
          el.style.display = "flex";
        } else {
          el.style.display = "none";
        }
      });
      if (!foundMatch) {
        foundString.style.display = "flex";
      } else {
        foundString.style.display = "none";
      }
    } else {
      elements.forEach((el) => {
        el.style.display = "flex";
      });
      foundString.style.display = "none";
    }
  });

  reset.addEventListener("click", function () {
    searchName.value = "";
    foundString.style.display = "none";
    reset.classList.add("no-reset");
    elements.forEach((el) => {
      el.style.display = "flex";
    });
  });
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/*************************************post***************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
const sendArray = [
  "xspeed",
  "yspeed",
  "zspeed",
  "xacc",
  "yacc",
  "zacc",
  "xdelay",
  "ydelay",
  "zdelay",
];

function motorCtrl() {
  if (isValid) {
    showLoadingAnimation("loading", "Sending Data...");
    const dataToSend = {};

    sendArray.forEach((key) => {
      if (key.includes("acc")) {
        const selectElement = document.querySelector(
          `select[name="${key}-write"]`
        );
        if (selectElement) {
          dataToSend[key] = selectElement.value;
        }
      } else {
        const inputElement = document.querySelector(
          `input[name="${key}-write"]`
        );
        if (inputElement) {
          dataToSend[key] = inputElement.value;
        }
      }
    });

    const xhr = new XMLHttpRequest();

    xhr.open("POST", "http://" + ip + "/op=w_2", true);
    xhr.setRequestHeader("Content-Type", "application/json");

    xhr.onload = function () {
      if (xhr.status >= 200 && xhr.status < 300) {
        setTimeout(() => {
          hideLoadingAnimation();
        }, 2000);
        setTimeout(() => {
          showLoadingAnimation("correct", "Success");
        }, 2100);
      } else {
        addLog(`XError /op=2: ${xhr.status}X`);
        setTimeout(() => {
          hideLoadingAnimation();
        }, 2000);
        setTimeout(() => {
          showLoadingAnimation(
            "warning",
            `${xhr.status}`
          );
        }, 2100);
      }
      setTimeout(() => {
        hideLoadingAnimation();
      }, 4100);
    };

    xhr.onerror = function () {
      addLog("XError of Connection /op=X");
      showLoadingAnimation("fail", `Error`);
      setTimeout(() => {
        hideLoadingAnimation();
      }, 2000);
    };
    xhr.send(JSON.stringify(dataToSend));
    addLog(`Vjson sent: ${JSON.stringify(dataToSend)}V`);
  } else {
    showLoadingAnimation("warning", "Error to validate input");
    setTimeout(() => {
      hideLoadingAnimation();
    }, 2000);
  }
}

function saveWifi() {
  let countWifi = document.querySelector("#count-wifi");
  let numCount = parseInt(countWifi.textContent, 10);

  showLoadingAnimation("loading", "Save Wifi...");

  const dataToSend = {};
  dataToSend.count = numCount;

  // Prendere tutti gli input in ordine dal DOM
  const ssidInputs = document.querySelectorAll("#wifi-list input[name^='ssid-']");
  const passInputs = document.querySelectorAll("#wifi-list input[name^='pass-']");

  // Controlla se gli input sono stati trovati
  console.log("SSID Inputs:", ssidInputs);
  console.log("PASS Inputs:", passInputs);

  ssidInputs.forEach((ssidInput, index) => {
    let passInput = passInputs[index];

    if (ssidInput && passInput && ssidInput.value.trim() !== "" && passInput.value.trim() !== "") {
      dataToSend[`ssid-${index + 1}`] = ssidInput.value.trim();
      dataToSend[`pass-${index + 1}`] = passInput.value.trim();

      console.log(`ssid-${index + 1}: ${ssidInput.value}`);
      console.log(`pass-${index + 1}: ${passInput.value}`);
    }
  });

  const xhr = new XMLHttpRequest();
  xhr.open("POST", `http://${ip}/op=wifi`, true);
  xhr.setRequestHeader("Content-Type", "application/json");

  xhr.onload = function () {
    if (xhr.status >= 200 && xhr.status < 300) {
      setTimeout(() => hideLoadingAnimation(), 2000);
      setTimeout(() => showLoadingAnimation("correct", "Success"), 2100);
    } else {
      addLog(`XError /op=wifi: ${xhr.status}X`);
      setTimeout(() => hideLoadingAnimation(), 2000);
      setTimeout(() => showLoadingAnimation("warning", `${xhr.status}`), 2100);
    }
    setTimeout(() => {
      hideLoadingAnimation();
      getOp2();
    }, 4100);
  };

  xhr.onerror = function () {
    addLog("XError of Connection /op=wifi");
    showLoadingAnimation("fail", `Error`);
    setTimeout(() => hideLoadingAnimation(), 2000);
  };

  xhr.send(JSON.stringify(dataToSend));

  addLog(`Vjson sent: ${JSON.stringify(dataToSend)}V`);
}

function openBanner() {
  document.querySelector(".banner-reset").style.display = "flex";
  document.querySelector("#layover").style.display = "block";
}

function closeBanner() {
  document.querySelector(".banner-reset").style.display = "none";
  document.querySelector("#layover").style.display = "none";
}

function restartCtrl() {
  closeBanner();
  showLoadingAnimation("loading", "Resetting µcontrol...")
  const xhr = new XMLHttpRequest();
  xhr.open("POST", "http://" + ip + "/op=restart", true);
  xhr.setRequestHeader("Content-Type", "application/json");

  xhr.onload = function () {
    if (xhr.status >= 200 && xhr.status < 300) {
      setTimeout(() => {
        hideLoadingAnimation();
      }, 2000);
      setTimeout(() => {
        showLoadingAnimation("correct", "Success");
      }, 2100);
    } else {
      addLog(`XError /op=2: ${xhr.status}X`);
      setTimeout(() => {
        hideLoadingAnimation();
      }, 2000);
      setTimeout(() => {
        showLoadingAnimation(
          "warning",
          `${xhr.status}`
        );
      }, 2100);
    }
    setTimeout(() => {
      hideLoadingAnimation();
      addLog("!restart µcontrol!");
      showLoadingAnimation("warning", "!Please reload the page!");
    }, 4100);
  };

  xhr.onerror = function () {
    addLog("XError of Connection /op=X");
    showLoadingAnimation("fail", `Error`);
    setTimeout(() => {
      hideLoadingAnimation();
    }, 2000);
  };
  xhr.send("restart");
  addLog(`Vjson sent: restartV`);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/*************************************loaded*************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

document.addEventListener("DOMContentLoaded", () => {
  /********************************************************************************/
  /********************************************************************************/
  /********************************************************************************/
  /*************************************wifi***************************************/
  /********************************************************************************/
  /********************************************************************************/
  /********************************************************************************/

  var wifi = document.querySelector("#wifi");
  var nowifi = document.querySelector("#nowifi");
  var infoWifi = document.querySelector("#info-wifi");

  async function isActuallyOnline() {
    try {
      await fetch('https://www.google.com', {
        method: 'HEAD',
        mode: 'no-cors',
        cache: 'no-store'
      });
      return true;
    } catch (error) {
      return false;
    }
  }

  function handleOnline() {
    console.log("online");
    wifi.style.display = "flex";
    nowifi.style.display = "none";
  }

  function handleOffline() {
    console.log("offline");
    wifi.style.display = "none";
    nowifi.style.display = "flex";
  }

  function updateNetworkInfo() {
    if ('connection' in navigator) {
      // Recupera i valori dalla Network Information API
      const rttValue = navigator.connection.rtt;
      const downlinkValue = navigator.connection.downlink;

      // Mostra i dati all’interno di #info-wifi
      infoWifi.innerHTML = `
        <div>${rttValue} ms</div>
        <div>${downlinkValue} Mbps</div>
      `;
    } else {
      infoWifi.innerHTML = `<p>Network Information API non disponibile.</p>`;
    }
  }

  setInterval(async () => {
    const online = await isActuallyOnline();
    if (online) {
      handleOnline();
    } else {
      handleOffline();
    }

    // Aggiorna le info di rete (se disponibili)
    updateNetworkInfo();
  }, 2500);

  /********************************************************************************/
  /********************************************************************************/
  /********************************************************************************/
  /**************************slide page con footer*********************************/
  /********************************************************************************/
  /********************************************************************************/
  /********************************************************************************/

  var items = [];
  var mains = [];
  var titles = [];
  let toggle = document.querySelector("#toggle-container");

  for (let i = 0; i < 5; i++) {
    items[i] = document.querySelector(`#item-${i}`);
    mains[i] = document.querySelector(`#main-${i}`);
    titles[i] = document.querySelector(`#title-${i}`);

    items[i].addEventListener("click", async function () {
      hideAllVisibleTooltips();
      hideAllMain();
      hideAllFooter();
      hideAllTitle();

      mains[i].classList.add("slide");
      items[i].classList.add("active-hover");
      titles[i].classList.add("slide-top");

      let asyncTask;
      switch (i) {
        case 0:
          toggle.style.display = "flex";
          stopRealTime();
          addLog("-----open arduino view-----");
          break;
        case 1:
          toggle.style.display = "none";
          stopRealTime();
          addLog("-----open arduino ctrl-----");
          break;
        case 2:
          toggle.style.display = "none";
          stopRealTime();
          addLog("-----open warehouse-----");
          break;
        case 3:
          toggle.style.display = "none";
          stopRealTime();
          addLog("-----open log-----");
          break;
        case 4:
          toggle.style.display = "none";
          stopRealTime();
          addLog("-----open qr code generator-----");
          break;
        default:
          addLog("-----item footer non gestito-----");
          break;
      }
    });
  }

  /********************************************************************************/
  /********************************************************************************/
  /********************************************************************************/
  /**************************main arduino-view*************************************/
  /********************************************************************************/
  /********************************************************************************/
  /********************************************************************************/

  function hideAllMain() {
    mains.forEach((main) => {
      main.classList.remove("slide");
    });
  }

  function hideAllFooter() {
    items.forEach((item) => {
      item.classList.remove("active-hover");
    });
  }

  function hideAllTitle() {
    titles.forEach((title) => {
      title.classList.remove("slide-top");
    });
  }

  document.querySelector("#item-0").addEventListener("click", getOp1);
  let intervalID = null;

  function startRealTime() {
    if (!intervalID) {
      intervalID = setInterval(toggleOp1, 1000);
    }
  }

  function stopRealTime() {
    toggleSwitch.checked = false;
    if (intervalID) {
      clearInterval(intervalID);
      intervalID = null;
    }
  }

  const toggleSwitch = document.querySelector("#real-time");

  toggleSwitch.addEventListener("change", function () {
    if (toggleSwitch.checked) {
      if (document.querySelector("#item-0").classList.contains("active-hover")) {
        startRealTime();
      }
    } else {
      stopRealTime();
    }
  });


  /********************************************************************************/
  /********************************************************************************/
  /********************************************************************************/
  /**********************************warehouse*************************************/
  /********************************************************************************/
  /********************************************************************************/
  /********************************************************************************/

  document.querySelectorAll(".face").forEach((face) => {
    const cube = face.closest(".cube");

    face.addEventListener("mouseover", () => {
      if (cube && !cube.classList.contains("full-box")) {
        cube.classList.add("full-cube");
      }
    });

    face.addEventListener("mouseout", () => {
      if (cube) {
        cube.classList.remove("full-cube");
      }
    });
  });

  document.querySelector("#item-2").addEventListener("click", getWarehouse);

  /********************************************************************************/
  /********************************************************************************/
  /********************************************************************************/
  /**********************************arduino-ctrl**********************************/
  /********************************************************************************/
  /********************************************************************************/
  /********************************************************************************/

  var motorButton = document.querySelector("#op_w2");
  var wifiButton = document.querySelector("#op_wifi");

  document.querySelector("#item-1").addEventListener("click", getOp2);

  motorButton.addEventListener("click", motorCtrl);

  wifiButton.addEventListener("click", saveWifi);

  document.querySelector("#open-banner").addEventListener("click", openBanner);

  document.querySelector("#close").addEventListener("click", closeBanner);

  document.querySelector("#restart").addEventListener("click", restartCtrl);

  /**************************drag****************************** */

  const wifiList = document.querySelector('#wifi-list');
  let draggedElement = null;

  wifiList.addEventListener('dragstart', (e) => {
    if (e.target.classList.contains('wifi-item')) {
      draggedElement = e.target;
      e.target.classList.add('dragging');
    }
  });

  wifiList.addEventListener('dragend', (e) => {
    if (e.target.classList.contains('wifi-item')) {
      e.target.classList.remove('dragging');
      draggedElement = null;
    }
  });

  wifiList.addEventListener('dragover', (e) => {
    e.preventDefault();
    const afterElement = getDragAfterElement(wifiList, e.clientY);
    if (afterElement == null) {
      wifiList.appendChild(draggedElement);
    } else {
      wifiList.insertBefore(draggedElement, afterElement);
    }
  });

  function getDragAfterElement(container, y) {
    const draggableElements = [...container.querySelectorAll('.wifi-item:not(.dragging)')];
    return draggableElements.reduce((closest, child) => {
      const box = child.getBoundingClientRect();
      const offset = y - box.top - box.height / 2;
      if (offset < 0 && offset > closest.offset) {
        return { offset: offset, element: child };
      } else {
        return closest;
      }
    }, { offset: Number.NEGATIVE_INFINITY }).element;
  }

  function scrollToBottom() {
    let container = document.getElementById("wifi-list");
    if (container) {
      container.scrollTo({
        top: container.scrollHeight,
        behavior: "smooth"
      });
    }
  }

  let addWifi = document.querySelector("#add-wifi");

  addWifi.addEventListener("click", function add() {
    let countWifi = document.querySelector("#count-wifi");
    let numCount = parseInt(countWifi.innerHTML);
    addWifiItem(numCount + 1, "", "");
    countWifi.innerHTML = numCount + 1;
    scrollToBottom();
  });

  /********************************************************************************/
  /********************************************************************************/
  /********************************************************************************/
  /**********************************validate**************************************/
  /********************************************************************************/
  /********************************************************************************/
  /********************************************************************************/
  var boxInputs = document.getElementsByClassName("input-arduino");
  var netInputs = document.getElementsByClassName("input-arduino-net");

  function validate(event) {
    var boxInput = event.target;
    var valueInput = parseFloat(boxInput.value);
    var regex = /^[0-9.,]+$/;

    if (
      !regex.test(boxInput.value) ||
      valueInput < 0.15 ||
      valueInput > 18.75
    ) {
      boxInput.style.borderColor = "crimson";
      boxInput.style.backgroundColor = "crimson";
      updateGlobalValidation();
      return;
    } else {
      boxInput.style.borderColor = "";
      boxInput.style.backgroundColor = "";
    }

    updateGlobalValidation();
  }

  function updateGlobalValidation() {
    isValid = true;
    for (let i = 0; i < boxInputs.length; i++) {
      var valueInput = parseFloat(boxInputs[i].value);
      var regex = /^[0-9.,]+$/;

      if (
        !regex.test(boxInputs[i].value) ||
        valueInput < 0.004 ||
        valueInput > 100
      ) {
        isValid = false;
        break;
      }
    }
  }

  for (let i = 0; i < boxInputs.length; i++) {
    boxInputs[i].addEventListener("input", validate);
  }

  /********************************************************************************/
  /********************************************************************************/
  /********************************************************************************/
  /***************************************log**************************************/
  /********************************************************************************/
  /********************************************************************************/
  /********************************************************************************/

  var containerLogText = document.querySelector("#log");
  var svgTrash = document.querySelector("#trash");
  var downLog = document.querySelector("#log-down");
  var downError = document.querySelector("#error-down");

  const observer = new MutationObserver(checkAndUpdate);
  checkAndUpdate();

  svgTrash.addEventListener("click", function () {
    setLog("");
    containerLogText.innerHTML = '<span style="width: 100%; height: 100%;" class="empty" eo-translator="empty log"></span>';
  });

  function checkAndUpdate() {
    if (containerLogText.innerHTML == '<span style="width: 100%; height: 100%;" class="empty" eo-translator="empty log"></span>') {
      svgTrash.classList.add("no-hover");
      downLog.classList.add("no-hover");
      downError.classList.add("no-hover");
    } else {
      svgTrash.classList.remove("no-hover");
      downLog.classList.remove("no-hover");
      downError.classList.remove("no-hover");
    }
  }

  observer.observe(containerLogText, {
    childList: true,
    subtree: true,
    characterData: true,
  });

  downLog.addEventListener("click", function () {
    const text = containerLogText.textContent;
    const blob = new Blob([text], { type: "text/plain" });
    const url = URL.createObjectURL(blob);
    const a = document.createElement("a");
    a.href = url;
    let newDateString = renameFile(getDate());
    a.download = `${newDateString}_log.txt`;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
  });

  downError.addEventListener("click", function () {
    var text = getError().replaceAll("<br>", "");
    if (text == "") {
      text = "-----no error-----";
    }
    const blob = new Blob([text], { type: "text/plain" });
    const url = URL.createObjectURL(blob);
    const a = document.createElement("a");
    a.href = url;
    let newDateString = renameFile(getDate());
    a.download = `${newDateString}_error.txt`;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
  });

  function renameFile(text) {
    let newText = text
      .replaceAll("<br>", "")
      .replaceAll("Date:", "")
      .replaceAll(";", "")
      .replaceAll("/", "")
      .replaceAll(":", "");
    return newText;
  }

  /********************************************************************************/
  /********************************************************************************/
  /********************************************************************************/
  /***********************************qr code**************************************/
  /********************************************************************************/
  /********************************************************************************/
  /********************************************************************************/

  function checkInputs() {
    const nameQr = document.querySelector("#name").value.trim();
    const codeQr = document.querySelector("#code").value.trim();
    const countQr = document.querySelector("#count").value.trim();

    const generateQr = document.querySelector("#generate-qr");
    const generateQrNone = document.querySelector("#generate-qr-none");

    const isCountValid = countQr !== "" && !isNaN(countQr) && Number(countQr) > 0;

    validateInput("#name", nameQr);
    validateInput("#code", codeQr);
    validateInput("#count", isCountValid);

    if (nameQr && codeQr && isCountValid) {
      generateQr.style.display = "flex";
      generateQrNone.style.display = "none";
    } else {
      generateQr.style.display = "none";
      generateQrNone.style.display = "flex";
    }
  }

  function validateInput(selector, condition) {
    const element = document.querySelector(selector);
    if (condition) {
      element.style.backgroundColor = "transparent";
      element.style.borderColor = "#ec7900";
    } else {
      element.style.backgroundColor = "crimson";
      element.style.borderColor = "crimson";
    }
  }

  function generateQRCode(data) {
    const qrCodeContainer = document.querySelector("#qr-code");
    qrCodeContainer.innerHTML = "";

    // Aggiungi stili per il bordo bianco
    qrCodeContainer.style.padding = "20px"; // Aggiungi padding per il bordo bianco
    qrCodeContainer.style.backgroundColor = "#ffffff"; // Imposta il colore di sfondo su bianco

    const qrCode = new QRCode(qrCodeContainer, {
      text: data,
      width: 170,
      height: 170,
      colorDark: "#000000",
      colorLight: "#ffffff",
      correctLevel: QRCode.CorrectLevel.H,
    });

    setTimeout(() => {
      document.querySelector("#download-qr").style.display = "flex";
      document.querySelector("#download-qr-none").style.display = "none";
    }, 500);
  }

  function downloadQRCode() {
    const qrCodeContainer = document.querySelector("#qr-code");
    const imgElement = qrCodeContainer.querySelector("img");

    if (imgElement) {
      // Crea un canvas per includere il bordo bianco
      const canvas = document.createElement("canvas");
      const ctx = canvas.getContext("2d");

      // Dimensioni del canvas (QR code + bordo)
      const borderSize = 20; // Dimensione del bordo
      const qrSize = 170; // Dimensione del QR code
      const canvasSize = qrSize + 2 * borderSize;

      canvas.width = canvasSize;
      canvas.height = canvasSize;

      // Riempie il canvas con il colore bianco
      ctx.fillStyle = "#ffffff";
      ctx.fillRect(0, 0, canvasSize, canvasSize);

      // Disegna il QR code al centro del canvas
      ctx.drawImage(imgElement, borderSize, borderSize, qrSize, qrSize);

      // Converte il canvas in un'immagine e la scarica
      canvas.toBlob((blob) => {
        saveAs(URL.createObjectURL(blob), "qrcode.png");
      });
    }
  }

  document.querySelector("#name").addEventListener("input", checkInputs);
  document.querySelector("#code").addEventListener("input", checkInputs);
  document.querySelector("#count").addEventListener("input", checkInputs);

  document.querySelector("#generate-qr").addEventListener("click", function () {
    const nameQr = document.querySelector("#name").value.trim();
    const codeQr = document.querySelector("#code").value.trim();
    const countQr = document.querySelector("#count").value.trim();

    const data = JSON.stringify({ name: nameQr, code: codeQr, count: countQr });
    generateQRCode(data);
  });

  document.querySelector("#download-qr").addEventListener("click", downloadQRCode);

  checkInputs();
});
