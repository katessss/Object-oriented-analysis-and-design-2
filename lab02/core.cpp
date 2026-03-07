#include <napi.h>
#include <string>
#include <vector>

// Экранирует строку для безопасной вставки в JSON
// Это решает проблему с кириллицей — UTF-8 байты проходят как есть
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

static CoffeeMachine g_machine;

Napi::Value MakeEspresso(const Napi::CallbackInfo& info) {
    int s = info.Length() > 0 ? info[0].As<Napi::Number>().Int32Value() : 0;
    return Napi::String::New(info.Env(), g_machine.make_espresso(s));
}
Napi::Value MakeAmericano(const Napi::CallbackInfo& info) {
    int s = info.Length() > 0 ? info[0].As<Napi::Number>().Int32Value() : 0;
    return Napi::String::New(info.Env(), g_machine.make_americano(s));
}
Napi::Value MakeCappuccino(const Napi::CallbackInfo& info) {
    int s = info.Length() > 0 ? info[0].As<Napi::Number>().Int32Value() : 0;
    return Napi::String::New(info.Env(), g_machine.make_cappuccino(s));
}
Napi::Value MakeLatte(const Napi::CallbackInfo& info) {
    int s = info.Length() > 0 ? info[0].As<Napi::Number>().Int32Value() : 0;
    return Napi::String::New(info.Env(), g_machine.make_latte(s));
}
Napi::Value GetStorage(const Napi::CallbackInfo& info) {
    return Napi::String::New(info.Env(), g_machine.get_storage_json());
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("makeEspresso",   Napi::Function::New(env, MakeEspresso));
    exports.Set("makeAmericano",  Napi::Function::New(env, MakeAmericano));
    exports.Set("makeCappuccino", Napi::Function::New(env, MakeCappuccino));
    exports.Set("makeLatte",      Napi::Function::New(env, MakeLatte));
    exports.Set("getStorage",     Napi::Function::New(env, GetStorage));
    return exports;
}

NODE_API_MODULE(coffee_machine, Init)
