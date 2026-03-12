#include "httplib.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;



// ХРАНИЛИЩЕ 
class Storage {
private:
    int water_ml;
    int beans_g;
    int milk_ml;
    int sugar_g;

public:
    Storage(int water, int beans, int milk, int sugar)
        : water_ml(water), beans_g(beans), milk_ml(milk), sugar_g(sugar) {}

    bool check_ingredients(int req_water, int req_beans, int req_milk, int req_sugar) const {
        if (water_ml < req_water) { cout << " [Ошибка] Не хватает воды!\n";         return false; }
        if (beans_g  < req_beans) { cout << " [Ошибка] Не хватает кофейных зерен!\n"; return false; }
        if (milk_ml  < req_milk)  { cout << " [Ошибка] Не хватает молока!\n";        return false; }
        if (sugar_g  < req_sugar) { cout << " [Ошибка] Не хватает сахара!\n";        return false; }
        return true;
    }

    void consume_water(int amount) { water_ml -= amount; }
    void consume_beans(int amount) { beans_g  -= amount; }
    void consume_milk (int amount) { milk_ml  -= amount; }
    void consume_sugar(int amount) { sugar_g  -= amount; }

    void refill_water() { water_ml = 1000; cout << " [Заправка] Вода пополнена до 1000мл\n"; }
    void refill_beans() { beans_g  = 200;  cout << " [Заправка] Зерна пополнены до 200г\n";  }
    void refill_milk()  { milk_ml  = 500;  cout << " [Заправка] Молоко пополнено до 500мл\n"; }
    void refill_sugar() { sugar_g  = 100;  cout << " [Заправка] Сахар пополнен до 100г\n";   }

    int get_water() const { return water_ml; }
    int get_beans() const { return beans_g;  }
    int get_milk()  const { return milk_ml;  }
    int get_sugar() const { return sugar_g;  }
};


// ПОДСИСТЕМЫ
class Grinder {
public:
    void grind(int amount_g, Storage& storage) {
        cout << " -> [Кофемолка] Мелем " << amount_g << "г зерен...\n";
        storage.consume_beans(amount_g);
    }
};

class Brewer {
public:
    void brew_espresso(int water_ml, Storage& storage) {
        cout << " -> [Заварочный узел] Пропускаем " << water_ml << "мл кипятка через кофе под давлением...\n";
        storage.consume_water(water_ml);
    }
};

class MilkFrother {
public:
    void froth_milk(int amount_ml, Storage& storage) {
        if (amount_ml > 0) {
            cout << " -> [Капучинатор] Нагреваем и взбиваем " << amount_ml << "мл молока...\n";
            storage.consume_milk(amount_ml);
        }
    }
};

class WaterHeater {
public:
    void dispense_hot_water(int amount_ml, Storage& storage) {
        if (amount_ml > 0) {
            cout << " -> [Бойлер] Добавляем " << amount_ml << "мл горячей воды...\n";
            storage.consume_water(amount_ml);
        }
    }
};

class SugarDispenser {
public:
    void add_sugar(int amount_g, Storage& storage) {
        if (amount_g > 0) {
            cout << " -> [Дозатор] Насыпаем " << amount_g << "г сахара...\n";
            storage.consume_sugar(amount_g);
        }
    }
};

// ФАСАД 
class CoffeeMachineFacade {
private:
    Storage storage;
    Grinder grinder;
    Brewer brewer;
    MilkFrother frother;
    WaterHeater water_heater;
    SugarDispenser sugar_dispenser;

public:
    CoffeeMachineFacade() : storage(1000, 200, 500, 100) {}

    void make_espresso() {
        cout << "\n=== Готовим Эспрессо ===\n";
        int water = 30, beans = 15;
        if (!storage.check_ingredients(water, beans, 0, 0)) return;
        grinder.grind(beans, storage);
        brewer.brew_espresso(water, storage);
        cout << "=== Эспрессо готов! ===\n";
    }

    void make_americano() {
        cout << "\n=== Готовим Американо ===\n";
        int water_for_coffee = 30, extra_water = 90, beans = 15;
        if (!storage.check_ingredients(water_for_coffee + extra_water, beans, 0, 0)) return;
        grinder.grind(beans, storage);
        brewer.brew_espresso(water_for_coffee, storage);
        water_heater.dispense_hot_water(extra_water, storage);
        cout << "=== Американо готов! ===\n";
    }

    void make_cappuccino() {
        cout << "\n=== Готовим Капучино ===\n";
        int water = 30, beans = 15, milk = 100;
        if (!storage.check_ingredients(water, beans, milk, 0)) return;
        grinder.grind(beans, storage);
        brewer.brew_espresso(water, storage);
        frother.froth_milk(milk, storage);
        cout << "=== Капучино готов! ===\n";
    }

    void make_latte() {
        cout << "\n=== Готовим Латте ===\n";
        int water = 30, beans = 15, milk = 150;
        if (!storage.check_ingredients(water, beans, milk, 0)) return;
        grinder.grind(beans, storage);
        brewer.brew_espresso(water, storage);
        frother.froth_milk(milk, storage);
        cout << "=== Латте готов! ===\n";
    }

    void add_sugar(int portions) {
        int total_sugar = portions * 5;
        if (!storage.check_ingredients(0, 0, 0, total_sugar)) return;
        sugar_dispenser.add_sugar(total_sugar, storage);
    }

    void refill_storage(const string& ingredient) {
        cout << "\n=== Пополнение: " << ingredient << " ===\n";
        if (ingredient == "water" || ingredient == "all") storage.refill_water();
        if (ingredient == "beans" || ingredient == "all") storage.refill_beans();
        if (ingredient == "milk"  || ingredient == "all") storage.refill_milk();
        if (ingredient == "sugar" || ingredient == "all") storage.refill_sugar();
    }


    int get_ingredient(const string& ingredient) const {
        if (ingredient == "water") return storage.get_water();
        if (ingredient == "beans") return storage.get_beans();
        if (ingredient == "milk")  return storage.get_milk();
        if (ingredient == "sugar") return storage.get_sugar();
        return -1; 
}
};


class CafeServer {
private:
    CoffeeMachineFacade machine; // Наша кофемашина (Фасад)
    httplib::Server server;   // Наш веб-сервер

    // Вспомогательный метод для формирования JSON состояния хранилища
    string storage_json() {
        return "{\"water\":" + to_string(machine.get_ingredient("water")) +
               ",\"beans\":" + to_string(machine.get_ingredient("beans")) +
               ",\"milk\":" + to_string(machine.get_ingredient("milk"))  +
               ",\"sugar\":" + to_string(machine.get_ingredient("sugar")) + "}";
    }

    // Вспомогательный метод для настройки CORS заголовков
    void set_cors(httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Content-Type", "application/json; charset=utf-8");
    }

public:
    CafeServer() {
        // Главная страница (отдает index.html)
        server.Get("/", [&](const httplib::Request&, httplib::Response& res) {
            ifstream file("index.html");
            if (file) {
                stringstream buffer;
                buffer << file.rdbuf();
                res.set_content(buffer.str(), "text/html");
            } else {
                res.status = 404;
                res.set_content("Файл index.html не найден!", "text/plain");
            }
        });

        // Получение текущих запасов
        server.Get("/storage", [&](const httplib::Request&, httplib::Response& res) {
            set_cors(res);
            res.set_content(storage_json(), "application/json");
        });

        //  Приготовление кофе
        server.Post("/brew", [&](const httplib::Request& req, httplib::Response& res) {
            set_cors(res);
            string body = req.body;
            string type;
            int sugar = 0;

            // Ручной парсинг JSON из тела запроса
            auto t_pos = body.find("\"type\"");
            if (t_pos != string::npos) {
                auto q1 = body.find('"', t_pos + 6);
                auto q2 = body.find('"', q1 + 1);
                if (q1 != string::npos && q2 != string::npos)
                    type = body.substr(q1 + 1, q2 - q1 - 1);
            }

            auto s_pos = body.find("\"sugar\"");
            if (s_pos != string::npos) {
                auto col = body.find(':', s_pos);
                if (col != string::npos)
                    sugar = stoi(body.substr(col + 1));
            }

            // Вызываем методы твоего Фасада
            if      (type == "espresso") machine.make_espresso();
            else if (type == "americano") machine.make_americano();
            else if (type == "cappuccino") machine.make_cappuccino();
            else if (type == "latte") machine.make_latte();
            
            // Если указан сахар, вызываем метод добавления сахара
            if (sugar > 0) machine.add_sugar(sugar);

            res.set_content("{\"ok\":true, \"storage\":" + storage_json() + "}", "application/json");
        });

        // Пополнение запасов
        server.Post("/refill", [&](const httplib::Request& req, httplib::Response& res) {
            set_cors(res);
            string ingredient = "all";
            auto i_pos = req.body.find("\"ingredient\"");
            if (i_pos != string::npos) {
                auto q1 = req.body.find('"', i_pos + 12);
                auto q2 = req.body.find('"', q1 + 1);
                if (q1 != string::npos && q2 != string::npos)
                    ingredient = req.body.substr(q1 + 1, q2 - q1 - 1);
            }

            machine.refill_storage(ingredient);
            res.set_content("{\"ok\":true, \"storage\":" + storage_json() + "}", "application/json");
        });

        // Обработка CORS (для браузеров)
        server.Options(".*", [&](const httplib::Request&, httplib::Response& res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type");
            res.status = 204;
        });
    }

    // Запуск сервера
    void run(const string& host, int port) {
        cout << "CafeOS Server (All-in-One) запущен на http://localhost:" << port << endl;
        server.listen(host.c_str(), port);
    }
};



int main() {
    CafeServer app;
    app.run("0.0.0.0", 8080);
    return 0;
}