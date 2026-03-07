#include "httplib.h"
#include <string>
#include <vector>
#include <iostream>

static std::string json_escape(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (unsigned char c : s) {
        if      (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else                out += c;
    }
    return out;
}

class Storage {
private:
    int water_ml, beans_g, milk_ml, sugar_g;
public:
    Storage(int water, int beans, int milk, int sugar)
        : water_ml(water), beans_g(beans), milk_ml(milk), sugar_g(sugar) {}

    std::string check_ingredients(int rw, int rb, int rm, int rs) const {
        if (water_ml < rw) return "Не хватает воды!";
        if (beans_g  < rb) return "Не хватает кофейных зерен!";
        if (milk_ml  < rm) return "Не хватает молока!";
        if (sugar_g  < rs) return "Не хватает сахара!";
        return "";
    }
    void consume_water(int a) { water_ml -= a; }
    void consume_beans(int a) { beans_g  -= a; }
    void consume_milk (int a) { milk_ml  -= a; }
    void consume_sugar(int a) { sugar_g  -= a; }
    int get_water() const { return water_ml; }
    int get_beans() const { return beans_g;  }
    int get_milk()  const { return milk_ml;  }
    int get_sugar() const { return sugar_g;  }
};

class Grinder {
public:
    std::string grind(int g, Storage& s) {
        s.consume_beans(g);
        return "Кофемолка: мелем " + std::to_string(g) + "г зерен";
    }
};

class Brewer {
public:
    std::string brew_espresso(int ml, Storage& s) {
        s.consume_water(ml);
        return "Заварочный узел: экстракция " + std::to_string(ml) + "мл";
    }
};

class MilkFrother {
public:
    std::string froth_milk(int ml, Storage& s) {
        if (ml > 0) { s.consume_milk(ml); return "Капучинатор: взбиваем " + std::to_string(ml) + "мл"; }
        return "";
    }
};

class WaterHeater {
public:
    std::string dispense_hot_water(int ml, Storage& s) {
        if (ml > 0) { s.consume_water(ml); return "Бойлер: горячая вода " + std::to_string(ml) + "мл"; }
        return "";
    }
};

class SugarDispenser {
public:
    std::string add_sugar(int g, Storage& s) {
        if (g > 0) { s.consume_sugar(g); return "Дозатор: сахар " + std::to_string(g) + "г"; }
        return "";
    }
};

class CoffeeMachine {
private:
    Storage        storage;
    Grinder        grinder;
    Brewer         brewer;
    MilkFrother    frother;
    WaterHeater    water_heater;
    SugarDispenser sugar_dispenser;

    std::string storage_json() {
        return "{\"water\":"  + std::to_string(storage.get_water()) +
               ",\"beans\":"  + std::to_string(storage.get_beans()) +
               ",\"milk\":"   + std::to_string(storage.get_milk())  +
               ",\"sugar\":"  + std::to_string(storage.get_sugar()) + "}";
    }

    std::string steps_to_json(const std::vector<std::string>& steps) {
        std::string json = "[";
        bool first = true;
        for (const auto& step : steps) {
            if (step.empty()) continue;
            if (!first) json += ",";
            json += "\"" + json_escape(step) + "\"";
            first = false;
        }
        return json + "]";
    }

    std::string ok(const std::string& name, const std::vector<std::string>& steps) {
        return "{\"ok\":true,\"name\":\"" + json_escape(name) + "\","
               "\"steps\":" + steps_to_json(steps) + ","
               "\"storage\":" + storage_json() + "}";
    }

    std::string err(const std::string& msg) {
        return "{\"ok\":false,\"error\":\"" + json_escape(msg) + "\","
               "\"storage\":" + storage_json() + "}";
    }

public:
    CoffeeMachine() : storage(1000, 200, 500, 100) {}

    std::string get_storage_json() { return storage_json(); }

    std::string refill_ingredient(const std::string& ingredient) {
        int w = storage.get_water(), b = storage.get_beans(), m = storage.get_milk(), s = storage.get_sugar();
        if (ingredient == "water" || ingredient == "all") w = 1000;
        if (ingredient == "beans" || ingredient == "all") b = 200;
        if (ingredient == "milk"  || ingredient == "all") m = 500;
        if (ingredient == "sugar" || ingredient == "all") s = 100;
        storage = Storage(w, b, m, s);
        return "{"ok":true,"storage":" + storage_json() + "}";
    }


    std::string make_espresso(int sp = 0) {
        int water = 30, beans = 15, sg = sp * 5;
        std::string e = storage.check_ingredients(water, beans, 0, sg);
        if (!e.empty()) return err(e);
        return ok("Эспрессо", { grinder.grind(beans, storage), brewer.brew_espresso(water, storage), sugar_dispenser.add_sugar(sg, storage) });
    }

    std::string make_americano(int sp = 0) {
        int wc = 30, we = 90, beans = 15, sg = sp * 5;
        std::string e = storage.check_ingredients(wc + we, beans, 0, sg);
        if (!e.empty()) return err(e);
        return ok("Американо", { grinder.grind(beans, storage), brewer.brew_espresso(wc, storage), water_heater.dispense_hot_water(we, storage), sugar_dispenser.add_sugar(sg, storage) });
    }

    std::string make_cappuccino(int sp = 0) {
        int water = 30, beans = 15, milk = 100, sg = sp * 5;
        std::string e = storage.check_ingredients(water, beans, milk, sg);
        if (!e.empty()) return err(e);
        return ok("Капучино", { grinder.grind(beans, storage), brewer.brew_espresso(water, storage), frother.froth_milk(milk, storage), sugar_dispenser.add_sugar(sg, storage) });
    }

    std::string make_latte(int sp = 0) {
        int water = 30, beans = 15, milk = 150, sg = sp * 5;
        std::string e = storage.check_ingredients(water, beans, milk, sg);
        if (!e.empty()) return err(e);
        return ok("Латте", { grinder.grind(beans, storage), brewer.brew_espresso(water, storage), frother.froth_milk(milk, storage), sugar_dispenser.add_sugar(sg, storage) });
    }
};

int main() {
    CoffeeMachine machine;
    httplib::Server server;

    auto set_cors = [](httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Content-Type", "application/json; charset=utf-8");
    };

    server.Get("/storage", [&](const httplib::Request&, httplib::Response& res) {
        set_cors(res);
        res.set_content(machine.get_storage_json(), "application/json");
    });

    server.Post("/brew", [&](const httplib::Request& req, httplib::Response& res) {
        set_cors(res);
        std::string type, body = req.body;
        int sugar = 0;

        auto t = body.find("\"type\"");
        if (t != std::string::npos) {
            auto q1 = body.find('"', t + 7);
            auto q2 = body.find('"', q1 + 1);
            if (q1 != std::string::npos && q2 != std::string::npos)
                type = body.substr(q1 + 1, q2 - q1 - 1);
        }

        auto s = body.find("\"sugar\"");
        if (s != std::string::npos) {
            auto col = body.find(':', s);
            if (col != std::string::npos)
                sugar = std::stoi(body.substr(col + 1));
        }

        std::string result;
        if      (type == "espresso")   result = machine.make_espresso(sugar);
        else if (type == "americano")  result = machine.make_americano(sugar);
        else if (type == "cappuccino") result = machine.make_cappuccino(sugar);
        else if (type == "latte")      result = machine.make_latte(sugar);
        else result = "{\"ok\":false,\"error\":\"Unknown type\"}";

        res.set_content(result, "application/json");
    });


    server.Post("/refill", [&](const httplib::Request& req, httplib::Response& res) {
        set_cors(res);
        std::string ingredient = "all", body = req.body;
        auto t = body.find(""ingredient"");
        if (t != std::string::npos) {
            auto q1 = body.find('"', t + 13);
            auto q2 = body.find('"', q1 + 1);
            if (q1 != std::string::npos && q2 != std::string::npos)
                ingredient = body.substr(q1 + 1, q2 - q1 - 1);
        }
        res.set_content(machine.refill_ingredient(ingredient), "application/json");
    });

    server.Options(".*", [&](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.status = 204;
    });

    std::cout << "CafeOS running at http://localhost:8080" << std::endl;
    server.listen("0.0.0.0", 8080);
    return 0;
}
