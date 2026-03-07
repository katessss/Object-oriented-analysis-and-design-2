const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');

// Загружаем скомпилированный C++ addon
const coffee = require('./build/Release/coffee_machine');

// ── Создаём окно приложения ──────────────────────────────────
function createWindow() {
  const win = new BrowserWindow({
    width: 960,
    height: 680,
    minWidth: 760,
    minHeight: 560,
    titleBarStyle: 'hiddenInset', // macOS: скрытый заголовок
    backgroundColor: '#0d0b09',
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'), // безопасный мост к C++
      contextIsolation: true,
      nodeIntegration: false,
    },
  });

  win.loadFile('index.html');
}

app.whenReady().then(createWindow);
app.on('window-all-closed', () => { if (process.platform !== 'darwin') app.quit(); });
app.on('activate', () => { if (BrowserWindow.getAllWindows().length === 0) createWindow(); });

// ── IPC-обработчики: вызовы из UI → C++ функции ─────────────
// Renderer отправляет 'brew', main получает и вызывает нужный метод C++

ipcMain.handle('getStorage', () => {
  return JSON.parse(coffee.getStorage());
});

ipcMain.handle('brew', (event, { type, sugar }) => {
  const sugarVal = sugar ?? 0;
  let result;

  switch (type) {
    case 'espresso':   result = coffee.makeEspresso(sugarVal);   break;
    case 'americano':  result = coffee.makeAmericano(sugarVal);  break;
    case 'cappuccino': result = coffee.makeCappuccino(sugarVal); break;
    case 'latte':      result = coffee.makeLatte(sugarVal);      break;
    default: return { ok: false, error: 'Неизвестный тип напитка' };
  }

  return JSON.parse(result); // C++ вернул JSON-строку → парсим в объект
});
