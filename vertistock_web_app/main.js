const { app, BrowserWindow, Menu } = require('electron');
const path = require('path');

let mainWindow;

app.on('ready', () => {
  mainWindow = new BrowserWindow({
    webPreferences: {},
    icon: path.join(__dirname, 'assets/icon.ico'),
    title: '', 
  });

  mainWindow.loadFile(path.join(__dirname, 'app', 'dist', 'index.html'));

  mainWindow.maximize();
  mainWindow.setMenuBarVisibility(false);
  Menu.setApplicationMenu(null);

  mainWindow.setTitle('');
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});
