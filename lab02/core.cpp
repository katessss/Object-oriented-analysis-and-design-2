#include <napi.h>
#include <string>
#include <sstream>

// ============================================================
//  Ваши оригинальные классы — БЕЗ ИЗМЕНЕНИЙ
// ============================================================

class Storage {
private:
    int water_ml;
    int beans_g;
    int milk_ml;
    int sugar_g;

public:
    Storage(int water, int beans, int milk, int sugar)
        : water_ml(water), beans_g(beans), milk_ml(milk), sugar_g(sugar) {}

    // Возвращает строку ошибки, или "" если всё ок
    std::string check_ingredients(int req_water, int req_beans, int req_milk, int req_sugar) const {
        if (water_ml < req_water) return "Не хватает воды!";
        if (beans_g  < req_beans) return "Не хватает кофейных зерен!";
        if (milk_ml  < req_milk)  return "Не хватает молока!";
        if (sugar_g  < req_sugar) return "Не хватает сахара!";
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
    std::string grind(int amount_g, Storage& storage) {
        storage.consume_beans(amount_g);
        return "Кофемолка: мелем " + std::to_string(amount_g) + "г зерен";
    }
};

class Brewer {
public:
    std::string brew_espresso(int water_ml, Storage& storage) {
        storage.consume_water(water_ml);
        return "Заварочный узел: пропускаем " + std::to_string(water_ml) + "мл кипятка";
    }
};

class MilkFrother {
public:
    std::string froth_milk(int amount_ml, Storage& storage) {
        if (amount_ml > 0) {
            storage.consume_milk(amount_ml);
            return "Капучинатор: взбиваем " + std::to_string(amount_ml) + "мл молока";
        }
        return "";
    }
};

class WaterHeater {
public:
    std::string dispense_hot_water(int amount_ml, Storage& storage) {
        if (amount_ml > 0) {
            storage.consume_water(amount_ml);
            return "Бойлер: добавляем " + std::to_string(amount_ml) + "мл горячей воды";
        }
        return "";
    }
};

class SugarDispenser {
public:
    std::string add_sugar(int amount_g, Storage& storage) {
        if (amount_g > 0) {
            storage.consume_sugar(amount_g);
            return "Дозатор: насыпаем " + std::to_string(amount_g) + "г сахара";
        }
        return "";
    }
};

// Фасад — тоже без изменений в логике
class CoffeeMachine {
private:
    Storage         storage;
    Grinder         grinder;
    Brewer          brewer;
    MilkFrother     frother;
    WaterHeater     water_heater;
    SugarDispenser  sugar_dispenser;

    // Вспомогательный метод: собирает шаги в JSON-массив строк
    std::string steps_to_json(const std::vector<std::string>& steps) {
        std::string json = "[";
        for (size_t i = 0; i < steps.size(); ++i) {
            if (steps[i].empty()) continue;
            json += "\"" + steps[i] + "\"";
            if (i + 1 < steps.size()) json += ",";
        }
        json += "]";
        return json;
    }

    // Формирует итоговый JSON-ответ
    std::string ok(const std::string& name, const std::vector<std::string>& steps) {
        return "{\"ok\":true,\"name\":\"" + name + "\",\"steps\":" + steps_to_json(steps) + ","
               "\"storage\":" + storage_json() + "}";
    }

    std::string err(const std::string& msg) {
        return "{\"ok\":false,\"error\":\"" + msg + "\",\"storage\":" + storage_json() + "}";
    }

public:
    CoffeeMachine() : storage(1000, 200, 500, 100) {}

    std::string storage_json() {
        return "{\"water\":" + std::to_string(storage.get_water()) +
               ",\"beans\":" + std::to_string(storage.get_beans()) +
               ",\"milk\":"  + std::to_string(storage.get_milk())  +
               ",\"sugar\":" + std::to_string(storage.get_sugar()) + "}";
    }

    std::string make_espresso(int sugar_portions = 0) {
        int water = 30, beans = 15;
        int sugar_g = sugar_portions * 5;
        std::string e = storage.check_ingredients(water, beans, 0, sugar_g);
        if (!e.empty()) return err(e);

        std::vector<std::string> steps = {
            grinder.grind(beans, storage),
            brewer.brew_espresso(water, storage),
            sugar_dispenser.add_sugar(sugar_g, storage)
        };
        return ok("Эспрессо", steps);
    }

    std::string make_americano(int sugar_portions = 0) {
        int water_coffee = 30, extra_water = 90, beans = 15;
        int sugar_g = sugar_portions * 5;
        std::string e = storage.check_ingredients(water_coffee + extra_water, beans, 0, sugar_g);
        if (!e.empty()) return err(e);

        std::vector<std::string> steps = {
            grinder.grind(beans, storage),
            brewer.brew_espresso(water_coffee, storage),
            water_heater.dispense_hot_water(extra_water, storage),
            sugar_dispenser.add_sugar(sugar_g, storage)
        };
        return ok("Американо", steps);
    }

    std::string make_cappuccino(int sugar_portions = 0) {
        int water = 30, beans = 15, milk = 100;
        int sugar_g = sugar_portions * 5;
        std::string e = storage.check_ingredients(water, beans, milk, sugar_g);
        if (!e.empty()) return err(e);

        std::vector<std::string> steps = {
            grinder.grind(beans, storage),
            brewer.brew_espresso(water, storage),
            frother.froth_milk(milk, storage),
            sugar_dispenser.add_sugar(sugar_g, storage)
        };
        return ok("Капучино", steps);
    }

    std::string make_latte(int sugar_portions = 0) {
        int water = 30, beans = 15, milk = 150;
        int sugar_g = sugar_portions * 5;
        std::string e = storage.check_ingredients(water, beans, milk, sugar_g);
        if (!e.empty()) return err(e);

        std::vector<std::string> steps = {
            grinder.grind(beans, storage),
            brewer.brew_espresso(water, storage),
            frother.froth_milk(milk, storage),
            sugar_dispenser.add_sugar(sugar_g, storage)
        };
        return ok("Латте", steps);
    }
};

// ============================================================
//  N-API обёртка — мост между C++ и JavaScript (Node.js)
//  Это единственный новый код, ~60 строк
// ============================================================

// Глобальный экземпляр машины (живёт пока запущен Electron)
static CoffeeMachine g_machine;

// Каждая функция принимает Napi::CallbackInfo и возвращает Napi::Value (строку JSON)
Napi::Value MakeEspresso(const Napi::CallbackInfo& info) {
    int sugar = info.Length() > 0 ? info[0].As<Napi::Number>().Int32Value() : 0;
    return Napi::String::New(info.Env(), g_machine.make_espresso(sugar));
}

Napi::Value MakeAmericano(const Napi::CallbackInfo& info) {
    int sugar = info.Length() > 0 ? info[0].As<Napi::Number>().Int32Value() : 0;
    return Napi::String::New(info.Env(), g_machine.make_americano(sugar));
}

Napi::Value MakeCappuccino(const Napi::CallbackInfo& info) {
    int sugar = info.Length() > 0 ? info[0].As<Napi::Number>().Int32Value() : 0;
    return Napi::String::New(info.Env(), g_machine.make_cappuccino(sugar));
}

Napi::Value MakeLatte(const Napi::CallbackInfo& info) {
    int sugar = info.Length() > 0 ? info[0].As<Napi::Number>().Int32Value() : 0;
    return Napi::String::New(info.Env(), g_machine.make_latte(sugar));
}

Napi::Value GetStorage(const Napi::CallbackInfo& info) {
    return Napi::String::New(info.Env(), g_machine.storage_json());
}

// Регистрируем функции как экспорты модуля
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("makeEspresso",   Napi::Function::New(env, MakeEspresso));
    exports.Set("makeAmericano",  Napi::Function::New(env, MakeAmericano));
    exports.Set("makeCappuccino", Napi::Function::New(env, MakeCappuccino));
    exports.Set("makeLatte",      Napi::Function::New(env, MakeLatte));
    exports.Set("getStorage",     Napi::Function::New(env, GetStorage));
    return exports;
}

NODE_API_MODULE(coffee_machine, Init)
