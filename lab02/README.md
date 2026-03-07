# CaféOS — C++ HTTP Backend
wget https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
Структура проекта:
```
cpp-coffee/
├── server.cpp    ← C++ логика + HTTP сервер
├── httplib.h     ← скачать отдельно (см. ниже)
└── index.html    ← UI (открыть в браузере)
```

---

## Установка и запуск

### 1. Скачать библиотеку httplib.h
```bash
wget https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
```

### 2. Скомпилировать и запустить сервер
```bash
g++ -o cafeos server.cpp -lpthread -std=c++11
./cafeos
```

### 3. Открыть UI в браузере
```bash
xdg-open index.html
# или просто открыть файл index.html в Firefox/Chrome
```

---

## API сервера

| Метод | URL | Описание |
|-------|-----|----------|
| GET | /storage | Текущие остатки ингредиентов |
| POST | /brew | Приготовить напиток |

Пример запроса:
```bash
curl -X POST http://localhost:8080/brew \
  -H "Content-Type: application/json" \
  -d '{"type":"cappuccino","sugar":2}'
```

Пример ответа:
```json
{
  "ok": true,
  "name": "Капучино",
  "steps": ["Кофемолка: мелем 15г", "Заварочный узел: экстракция 30мл", "Капучинатор: взбиваем 100мл"],
  "storage": {"water": 940, "beans": 170, "milk": 400, "sugar": 90}
}
```
