const { contextBridge, ipcRenderer } = require('electron');

// Безопасно открываем только нужные функции для UI (index.html)
// UI не имеет прямого доступа к Node.js — только через этот мост
contextBridge.exposeInMainWorld('coffeeMachine', {
  getStorage: ()             => ipcRenderer.invoke('getStorage'),
  brew:       (type, sugar)  => ipcRenderer.invoke('brew', { type, sugar }),
});
