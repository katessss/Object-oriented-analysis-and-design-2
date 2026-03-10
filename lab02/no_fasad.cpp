#include "httplib.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;




class Storage {
private:
    int water_ml, beans_g, milk_ml, sugar_g;
public:
    Storage(int w, int b, int m, int s) : water_ml(w), beans_g(b), milk_ml(m), sugar_g(s) {}

    void consume_water(int a) { water_ml -= a; }
    void consume_beans(int a) { beans_g -= a;  }
    void consume_milk(int a)  { milk_ml -= a;  }
    void consume_sugar(int a) { sugar_g -= a;  }

    void refill_water() { water_ml = 1000; cout << "[Заправка] Вода\n"; }
    void refill_beans() { beans_g  = 200;  cout << "[Заправка] Зерна\n"; }
    void refill_milk()  { milk_ml  = 500;  cout << "[Заправка] Молоко\n"; }
    void refill_sugar() { sugar_g  = 100;  cout << "[Заправка] Сахар\n"; }

    int get_water() const { return water_ml; }
    int get_beans() const { return beans_g; }
    int get_milk()  const { return milk_ml; }
    int get_sugar() const { return sugar_g; }
};

class Grinder {
public:
    string grind(int g, Storage& s) {
        s.consume_beans(g);
        return "[Кофемолка] Мелем " + to_string(g) + "г зерен";
    }
};

class Brewer {
public:
    string brew(int w, Storage& s) {
        s.consume_water(w);
        return "[Заварочный узел] Экстракция " + to_string(w) + "мл";
    }
};

class MilkFrother {
public:
    string froth(int m, Storage& s) {
        s.consume_milk(m);
        return "[Капучинатор] Взбиваем " + to_string(m) + "мл молока";
    }
};

class WaterHeater {
public:
    string heat(int w, Storage& s) {
        s.consume_water(w);
        return "[Бойлер] Добавляем " + to_string(w) + "мл кипятка";
    }
};

class SugarDispenser {
public:
    string add(int g, Storage& s) {
        s.consume_sugar(g);
        return "[Дозатор] Насыпаем " + to_string(g) + "г сахара";
    }
};

// Утилиты для  JSON
static string json_escape(const string& s) {
    string out;
    for (unsigned char c : s) {
        if      (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else                out += c;
    }
    return out;
}

string get_storage_json(const Storage& s) {
    return "{\"water\":" + to_string(s.get_water()) +
           ",\"beans\":" + to_string(s.get_beans()) +
           ",\"milk\":"  + to_string(s.get_milk()) +
           ",\"sugar\":" + to_string(s.get_sugar()) + "}";
}

string make_error(const string& msg, const Storage& s) {
    return "{\"ok\":false,\"error\":\"" + json_escape(msg) + "\",\"storage\":" + get_storage_json(s) + "}";
}

string make_success(const string& name, const vector<string>& steps, const Storage& s) {
    string json = "{\"ok\":true,\"name\":\"" + json_escape(name) + "\",\"steps\":[";
    for (size_t i = 0; i < steps.size(); ++i) {
        json += "\"" + json_escape(steps[i]) + "\"" + (i == steps.size() - 1 ? "" : ",");
    }
    json += "],\"storage\":" + get_storage_json(s) + "}";
    return json;
}

// MAIN - СЕРВЕР (играет роль клиента, который знает всё)
int main() {
    // Создаем все механизмы вручную прямо в main
    Storage storage(1000, 200, 500, 100);
    Grinder grinder;
    Brewer brewer;
    MilkFrother frother;
    WaterHeater heater;
    SugarDispenser sugar_dispenser;

    httplib::Server server;

    auto set_cors = [](httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Content-Type", "application/json; charset=utf-8");
    };

    // --- GET /storage ---
    server.Get("/storage", [&](const httplib::Request&, httplib::Response& res) {
        set_cors(res);
        res.set_content(get_storage_json(storage), "application/json");
    });

    // --- POST /brew ---
    server.Post("/brew", [&](const httplib::Request& req, httplib::Response& res) {
        set_cors(res);
        string type, body = req.body;
        int sugar_portions = 0;

        // Ручной парсинг JSON
        auto t = body.find("\"type\"");
        if (t != string::npos) {
            auto q1 = body.find('"', t + 7);
            auto q2 = body.find('"', q1 + 1);
            if (q1 != string::npos && q2 != string::npos) type = body.substr(q1 + 1, q2 - q1 - 1);
        }
        auto s_pos = body.find("\"sugar\"");
        if (s_pos != string::npos) {
            auto col = body.find(':', s_pos);
            if (col != string::npos) sugar_portions = stoi(body.substr(col + 1));
        }

        int s_grams = sugar_portions * 5;
        vector<string> steps;
        string result;

        // логика
        if (type == "espresso") {
            if (storage.get_water() < 30 || storage.get_beans() < 15 || storage.get_sugar() < s_grams) {
                result = make_error("Не хватает ингредиентов!", storage);
            } else {
                steps.push_back(grinder.grind(15, storage));
                steps.push_back(brewer.brew(30, storage));
                if (s_grams > 0) steps.push_back(sugar_dispenser.add(s_grams, storage));
                result = make_success("Эспрессо", steps, storage);
            }
        } 
        else if (type == "americano") {
            if (storage.get_water() < 120 || storage.get_beans() < 15 || storage.get_sugar() < s_grams) {
                result = make_error("Не хватает ингредиентов!", storage);
            } else {
                steps.push_back(grinder.grind(15, storage));
                steps.push_back(brewer.brew(30, storage));
                steps.push_back(heater.heat(90, storage));
                if (s_grams > 0) steps.push_back(sugar_dispenser.add(s_grams, storage));
                result = make_success("Американо", steps, storage);
            }
        }
        else if (type == "cappuccino") {
            if (storage.get_water() < 30 || storage.get_beans() < 15 || storage.get_milk() < 100 || storage.get_sugar() < s_grams) {
                result = make_error("Не хватает ингредиентов!", storage);
            } else {
                steps.push_back(grinder.grind(15, storage));
                steps.push_back(brewer.brew(30, storage));
                steps.push_back(frother.froth(100, storage));
                if (s_grams > 0) steps.push_back(sugar_dispenser.add(s_grams, storage));
                result = make_success("Капучино", steps, storage);
            }
        }
        else if (type == "latte") {
            if (storage.get_water() < 30 || storage.get_beans() < 15 || storage.get_milk() < 150 || storage.get_sugar() < s_grams) {
                result = make_error("Не хватает ингредиентов!", storage);
            } else {
                steps.push_back(grinder.grind(15, storage));
                steps.push_back(brewer.brew(30, storage));
                steps.push_back(frother.froth(150, storage));
                if (s_grams > 0) steps.push_back(sugar_dispenser.add(s_grams, storage));
                result = make_success("Латте", steps, storage);
            }
        } 
        else {
            result = make_error("Неизвестный напиток", storage);
        }

        cout << "Выполнен запрос на приготовление: " << type << "\n";
        res.set_content(result, "application/json");
    });

    // --- POST /refill ---
    server.Post("/refill", [&](const httplib::Request& req, httplib::Response& res) {
        set_cors(res);
        string ingredient = "all", body = req.body;
        auto t = body.find("\"ingredient\"");
        if (t != string::npos) {
            auto q1 = body.find('"', t + 13);
            auto q2 = body.find('"', q1 + 1);
            if (q1 != string::npos && q2 != string::npos) ingredient = body.substr(q1 + 1, q2 - q1 - 1);
        }

        if (ingredient == "water" || ingredient == "all") storage.refill_water();
        if (ingredient == "beans" || ingredient == "all") storage.refill_beans();
        if (ingredient == "milk"  || ingredient == "all") storage.refill_milk();
        if (ingredient == "sugar" || ingredient == "all") storage.refill_sugar();

        res.set_content("{\"ok\":true,\"storage\":" + get_storage_json(storage) + "}", "application/json");
    });

    // --- OPTIONS (CORS) ---
    server.Options(".*", [&](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.status = 204;
    });

    cout << "Сервер (БЕЗ ФАСАДА) запущен на http://localhost:8080\n";
    server.listen("0.0.0.0", 8080);
    return 0;
}