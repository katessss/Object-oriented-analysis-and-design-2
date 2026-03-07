# CaféOS — Electron + C++ Desktop App

Структура проекта:

```
electron-coffee/
├── core.cpp        ← ваш C++ (модифицированный для N-API)
├── binding.gyp     ← конфиг сборки C++ addon
├── main.js         ← Electron: создаёт окно, обрабатывает вызовы C++
├── preload.js      ← безопасный мост UI ↔ C++
├── index.html      ← UI (вызывает реальные C++ функции)
└── package.json
```

---

## Установка и запуск

### 1. Установить зависимости
```bash
npm install
```

### 2. Скомпилировать C++ addon
```bash
npm run build-addon
```
> Требует: Python 3, C++ компилятор  
> macOS: `xcode-select --install`  
> Windows: `npm install --global windows-build-tools`  
> Linux: `sudo apt install build-essential`

### 3. Запустить приложение
```bash
npm start
```

Или обе команды сразу:
```bash
npm run dev
```

---

## Как это работает

```
[ index.html ]
    ↓ window.coffeeMachine.brew('cappuccino', 2)
[ preload.js ]
    ↓ ipcRenderer.invoke('brew', { type, sugar })
[ main.js ]
    ↓ coffee.makeCappuccino(2)   ← вызов C++ функции
[ core.cpp ]
    → CoffeeMachine::make_cappuccino(2)
    → возвращает JSON: { ok, name, steps, storage }
    ↑ обратно в UI
[ index.html ]
    → анимирует шаги, обновляет бары из реального C++ состояния
```

---

## Что изменилось в core.cpp

1. Добавлен `#include <napi.h>` — заголовок N-API
2. Методы теперь возвращают `std::string` (JSON) вместо `cout`
3. Добавлены ~60 строк N-API обёртки внизу файла
4. Вся логика (`Storage`, `Grinder`, `Brewer` и т.д.) — **без изменений**
