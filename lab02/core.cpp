#include "httplib.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

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


class Storage {
private:
    int water_ml;
    int beans_g;
    int milk_ml;
    int sugar_g;

public:
    Storage(int water, int beans, int milk, int sugar): water_ml(water), beans_g(beans), milk_ml(milk), sugar_g(sugar) {}

    bool check_ingredients(int req_water, int req_beans, int req_milk, int req_sugar) const {
        if (water_ml < req_water) { cout << " [Ошибка] Не хватает воды!\n"; return false; }
        if (beans_g < req_beans)  { cout << " [Ошибка] Не хватает кофейных зерен!\n"; return false; }
        if (milk_ml < req_milk)   { cout << " [Ошибка] Не хватает молока!\n"; return false; }
        if (sugar_g < req_sugar)  { cout << " [Ошибка] Не хватает сахара!\n"; return false; }
        return true;
    }

    void consume_water(int amount) { water_ml -= amount; }
    void consume_beans(int amount) { beans_g  -= amount; }
    void consume_milk(int amount)  { milk_ml  -= amount; }
    void consume_sugar(int amount) { sugar_g  -= amount; }

    // Пополнение резервуаров до максимума
    void refill_water()  { water_ml = 1000; cout << " [Заправка] Вода пополнена до 1000мл\n"; }
    void refill_beans()  { beans_g  = 200;  cout << " [Заправка] Зерна пополнены до 200г\n"; }
    void refill_milk()   { milk_ml  = 500;  cout << " [Заправка] Молоко пополнено до 500мл\n"; }
    void refill_sugar()  { sugar_g  = 100;  cout << " [Заправка] Сахар пополнен до 100г\n"; }

    void print_status() const {
        cout << "\n--- Остатки ингредиентов ---\n";
        cout << "Вода:  " << water_ml << " мл\n";
        cout << "Зерна: " << beans_g  << " г\n";
        cout << "Молоко:" << milk_ml  << " мл\n";
        cout << "Сахар: " << sugar_g  << " г\n";
        cout << "----------------------------\n\n";
    }

    int get_water() const { return water_ml; }
    int get_beans() const { return beans_g;  }
    int get_milk()  const { return milk_ml;  }
    int get_sugar() const { return sugar_g;  }
};


class Grinder {
public:
    string grind(int amount_g, Storage& storage) {
        string msg = " -> [Кофемолка] Мелем " + to_string(amount_g) + "г зерен...";
        cout << msg << "\n";
        storage.consume_beans(amount_g);
        return msg;
    }
};

class Brewer {
public:
    string brew_espresso(int water_ml, Storage& storage) {
        string msg = " -> [Заварочный узел] Пропускаем " + to_string(water_ml) + "мл кипятка через кофе под давлением...";
        cout << msg << "\n";
        storage.consume_water(water_ml);
        return msg;
    }
};

class MilkFrother {
public:
    string froth_milk(int amount_ml, Storage& storage) {
        if (amount_ml > 0) {
            string msg = " -> [Капучинатор] Нагреваем и взбиваем " + to_string(amount_ml) + "мл молока...";
            cout << msg << "\n";
            storage.consume_milk(amount_ml);
            return msg;
        }
        return "";
    }
};

class WaterHeater {
public:
    string dispense_hot_water(int amount_ml, Storage& storage) {
        if (amount_ml > 0) {
            string msg = " -> [Бойлер] Добавляем " + to_string(amount_ml) + "мл горячей воды...";
            cout << msg << "\n";
            storage.consume_water(amount_ml);
            return msg;
        }
        return "";
    }
};

class SugarDispenser {
public:
    string add_sugar(int amount_g, Storage& storage) {
        if (amount_g > 0) {
            string msg = " -> [Дозатор] Насыпаем " + to_string(amount_g) + "г сахара...";
            cout << msg << "\n";
            storage.consume_sugar(amount_g);
            return msg;
        }
        return "";
    }
};

class CoffeeMachineFacade {
private:
    Storage        storage;
    Grinder        grinder;
    Brewer         brewer;
    MilkFrother    frother;
    WaterHeater    water_heater;
    SugarDispenser sugar_dispenser;

    string storage_json() {
        return "{\"water\":"  + to_string(storage.get_water()) +
               ",\"beans\":"  + to_string(storage.get_beans()) +
               ",\"milk\":"   + to_string(storage.get_milk())  +
               ",\"sugar\":"  + to_string(storage.get_sugar()) + "}";
    }

    string steps_to_json(const vector<string>& steps) {
        string json = "[";
        bool first = true;
        for (const auto& step : steps) {
            if (step.empty()) continue;
            if (!first) json += ",";
            json += "\"" + json_escape(step) + "\"";
            first = false;
        }
        return json + "]";
    }

    string ok(const string& name, const vector<string>& steps) {
        return "{\"ok\":true,\"name\":\"" + json_escape(name) + "\","
               "\"steps\":"   + steps_to_json(steps) + ","
               "\"storage\":" + storage_json() + "}";
    }

    string err(const string& msg) {
        return "{\"ok\":false,\"error\":\"" + json_escape(msg) + "\","
               "\"storage\":" + storage_json() + "}";
    }

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

    void show_storage_status() const {
        storage.print_status();
    }


    string http_make_espresso(int sugar_portions = 0) {
        cout << "\n=== Готовим Эспрессо ===\n";
        int water = 30, beans = 15, sg = sugar_portions * 5;
        if (!storage.check_ingredients(water, beans, 0, sg))
            return err("Не хватает ингредиентов!");
        return ok("Эспрессо", {
            grinder.grind(beans, storage),
            brewer.brew_espresso(water, storage),
            sugar_dispenser.add_sugar(sg, storage)
        });
    }

    string http_make_americano(int sugar_portions = 0) {
        cout << "\n=== Готовим Американо ===\n";
        int wc = 30, we = 90, beans = 15, sg = sugar_portions * 5;
        if (!storage.check_ingredients(wc + we, beans, 0, sg))
            return err("Не хватает ингредиентов!");
        return ok("Американо", {
            grinder.grind(beans, storage),
            brewer.brew_espresso(wc, storage),
            water_heater.dispense_hot_water(we, storage),
            sugar_dispenser.add_sugar(sg, storage)
        });
    }

    string http_make_cappuccino(int sugar_portions = 0) {
        cout << "\n=== Готовим Капучино ===\n";
        int water = 30, beans = 15, milk = 100, sg = sugar_portions * 5;
        if (!storage.check_ingredients(water, beans, milk, sg))
            return err("Не хватает ингредиентов!");
        return ok("Капучино", {
            grinder.grind(beans, storage),
            brewer.brew_espresso(water, storage),
            frother.froth_milk(milk, storage),
            sugar_dispenser.add_sugar(sg, storage)
        });
    }

    string http_make_latte(int sugar_portions = 0) {
        cout << "\n=== Готовим Латте ===\n";
        int water = 30, beans = 15, milk = 150, sg = sugar_portions * 5;
        if (!storage.check_ingredients(water, beans, milk, sg))
            return err("Не хватает ингредиентов!");
        return ok("Латте", {
            grinder.grind(beans, storage),
            brewer.brew_espresso(water, storage),
            frother.froth_milk(milk, storage),
            sugar_dispenser.add_sugar(sg, storage)
        });
    }


    string refill(const string& ingredient) {
        cout << "\n=== Пополнение резервуара: " << ingredient << " ===\n";
        if (ingredient == "water" || ingredient == "all") storage.refill_water();
        if (ingredient == "beans" || ingredient == "all") storage.refill_beans();
        if (ingredient == "milk"  || ingredient == "all") storage.refill_milk();
        if (ingredient == "sugar" || ingredient == "all") storage.refill_sugar();
        return "{\"ok\":true,\"storage\":" + storage_json() + "}";
    }

    string get_storage_json() { return storage_json(); }
};

int main() {
    CoffeeMachineFacade machine;
    httplib::Server server;

    auto set_cors = [](httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Content-Type", "application/json; charset=utf-8");
    };

    // GET /storage
    server.Get("/storage", [&](const httplib::Request&, httplib::Response& res) {
        set_cors(res);
        res.set_content(machine.get_storage_json(), "application/json");
    });

    // POST /brew  {"type":"cappuccino","sugar":2}
    server.Post("/brew", [&](const httplib::Request& req, httplib::Response& res) {
        set_cors(res);
        string type, body = req.body;
        int sugar = 0;

        auto t = body.find("\"type\"");
        if (t != string::npos) {
            auto q1 = body.find('"', t + 7);
            auto q2 = body.find('"', q1 + 1);
            if (q1 != string::npos && q2 != string::npos)
                type = body.substr(q1 + 1, q2 - q1 - 1);
        }
        auto s = body.find("\"sugar\"");
        if (s != string::npos) {
            auto col = body.find(':', s);
            if (col != string::npos)
                sugar = stoi(body.substr(col + 1));
        }

        string result;
        if      (type == "espresso")   result = machine.http_make_espresso(sugar);
        else if (type == "americano")  result = machine.http_make_americano(sugar);
        else if (type == "cappuccino") result = machine.http_make_cappuccino(sugar);
        else if (type == "latte")      result = machine.http_make_latte(sugar);
        else result = "{\"ok\":false,\"error\":\"Unknown type\"}";

        res.set_content(result, "application/json");
    });

    // POST /refill  {"ingredient":"water"}
    server.Post("/refill", [&](const httplib::Request& req, httplib::Response& res) {
        set_cors(res);
        string ingredient = "all", body = req.body;

        auto t = body.find("\"ingredient\"");
        if (t != string::npos) {
            auto q1 = body.find('"', t + 13);
            auto q2 = body.find('"', q1 + 1);
            if (q1 != string::npos && q2 != string::npos)
                ingredient = body.substr(q1 + 1, q2 - q1 - 1);
        }

        res.set_content(machine.refill(ingredient), "application/json");
    });

    // OPTIONS (CORS preflight)
    server.Options(".*", [&](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.status = 204;
    });

    cout << "CafeOS running at http://localhost:8080\n";
    server.listen("0.0.0.0", 8080);
    return 0;
}
