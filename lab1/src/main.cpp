#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <algorithm>
#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

// ===================== MODEL =====================
// Интерфейс наблюдателя: активная модель сама уведомляет представление
class IObserver {
public:
    virtual ~IObserver() = default;
    virtual void update() = 0;
};

class Model {
public:
    void addObserver(IObserver* o) { observers.push_back(o); }

    // Проверка корректности даты (с учётом високосных лет)
    static bool isLeap(int y) { return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0; }
    static int daysInMonth(int m, int y) {
        static const int d[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
        return (m == 2 && isLeap(y)) ? 29 : d[m - 1];
    }
    static bool isValidDate(int d, int m, int y) {
        if (y < 1583 || y > 2026) return false;   // григорианский календарь
        if (m < 1 || m > 12) return false;
        return d >= 1 && d <= daysInMonth(m, y);
    }

    // Алгоритм Сакамото: 0 - воскресенье, 1 - понедельник, ... 6 - суббота
    static int weekday(int d, int m, int y) {
        static const int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
        if (m < 3) y -= 1;
        return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
    }

    static const char* dayName(int w) {
        static const char* n[] = { "Воскресенье", "Понедельник", "Вторник", "Среда",
                                   "Четверг", "Пятница", "Суббота" };
        return n[w];
    }
    static const char* destiny(int w) {
        static const char* s[] = {
            "Воскресенье (покровитель - Солнце). Судьба яркого лидера: вас ждут успех, "
            "признание и слава. Вы щедры, оптимистичны и умеете вести за собой людей.",
            "Понедельник (покровитель - Луна). Судьба мечтателя: вы чувствительны, "
            "интуитивны и привязаны к семье. Вам сопутствует удача в делах, связанных "
            "с домом, заботой и творчеством.",
            "Вторник (покровитель - Марс). Судьба борца: вы энергичны, смелы и упорны. "
            "Вам предстоит многого добиться в спорте, технике и деле, требующем решимости.",
            "Среда (покровитель - Меркурий). Судьба коммуникатора: вы любознательны, "
            "остроумны и легко учитесь. Вас ждёт успех в торговле, науке и путешествиях.",
            "Четверг (покровитель - Юпитер). Судьба мудреца: вам благоволят удача и "
            "справедливость. Вас ждёт карьерный рост и уважение окружающих.",
            "Пятница (покровитель - Венера). Судьба ценителя красоты: вы обаятельны, "
            "любите искусство и гармонию. Вас ждут счастье в любви и достаток.",
            "Суббота (покровитель - Сатурн). Судьба труженика: вы терпеливы, ответственны "
            "и дисциплинированны. Ваш путь долог, но награда за труд будет заслуженной."
        };
        return s[w];
    }

    // Установка данных: после изменения модель уведомляет наблюдателей
    void setDate(int d, int m, int y) {
        day = d; month = m; year = y;
        weekDay = weekday(d, m, y);
        hasData = true;
        notify();
    }

    int getDay() const { return day; }
    int getMonth() const { return month; }
    int getYear() const { return year; }
    int getWeekDay() const { return weekDay; }
    bool isEmpty() const { return !hasData; }

private:
    void notify() { for (auto* o : observers) o->update(); }

    int day = 0, month = 0, year = 0, weekDay = 0;
    bool hasData = false;
    vector<IObserver*> observers;
};

// ===================== VIEW =====================
class View : public IObserver {
public:
    explicit View(Model& m) : model(m) { model.addObserver(this); }

    // Вызывается моделью при изменении данных
    void update() override { show(); }

    void show() const {
        cout << "\n==================================================\n";
        cout << "       АСТРОЛОГ: СУДЬБА ПО ДНЮ НЕДЕЛИ РОЖДЕНИЯ\n";
        cout << "==================================================\n";
        if (model.isEmpty()) {
            cout << "Данные ещё не вводились.\n";
        }
        else {
            cout << "Последняя введённая дата: " << pad(model.getDay()) << "."
                << pad(model.getMonth()) << "." << model.getYear() << "\n";
            cout << "День недели рождения: " << Model::dayName(model.getWeekDay()) << "\n\n";
            cout << "Ваша судьба:\n" << Model::destiny(model.getWeekDay()) << "\n";
        }
        cout << "--------------------------------------------------\n";
    }

    void showMenu() const {
        cout << "1. Ввести дату рождения\n0. Выход\nВыбор: ";
    }
    void showError(const string& msg) const { cout << "Ошибка! " << msg << "\n"; }

    // Форма ввода (стандартный элемент ввода даты не используется)
    void showInputForm(int d, int m, int y) const {
        cout << "\n--- ВВОД ДАТЫ РОЖДЕНИЯ ---\n";
        if (d != 0)
            cout << "(предыдущий ввод: " << pad(d) << "." << pad(m) << "." << y << ")\n";
    }

private:
    static string pad(int v) { return (v < 10 ? "0" : "") + to_string(v); }
    Model& model;
};

// ===================== CONTROLLER =====================
class Controller {
public:
    Controller(Model& m, View& v) : model(m), view(v) {}

    void run() {
        view.show();
        bool running = true;
        while (running) {
            view.showMenu();
            int choice;
            if (!readInt(choice)) { view.showError("Введите 0 или 1."); continue; }
            if (choice == 1) handleInput();
            else if (choice == 0) running = false;
            else view.showError("Введите 0 или 1.");
        }
        cout << "\nДо свидания!\n";
    }

private:
    // Ввод числа с проверкой; false при нечисловом вводе
    bool readInt(int& v) {
        string s;
        if (!getline(cin, s)) { v = 0; return true; }
        if (s.empty() || s.size() > 6) return false;
        for (char c : s) if (!isdigit(static_cast<unsigned char>(c))) return false;
        v = stoi(s);
        return true;
    }

    void handleInput() {
        // Восстановление последних введённых данных
        int ld = model.isEmpty() ? 0 : model.getDay();
        int lm = model.isEmpty() ? 0 : model.getMonth();
        int ly = model.isEmpty() ? 0 : model.getYear();
        view.showInputForm(ld, lm, ly);

        int d, m, y;
        cout << "День: ";   bool ok = readInt(d);
        cout << "Месяц: ";  ok = readInt(m) && ok;
        cout << "Год: ";    ok = readInt(y) && ok;

        if (!ok) { view.showError("Дата должна состоять из чисел."); return; }
        if (!Model::isValidDate(d, m, y)) {
            view.showError("Такой даты не существует (год 1583-2026, "
                "месяц 1-12, день по календарю).");
            return;
        }
        model.setDate(d, m, y);   // модель сама уведомит представление
    }

    Model& model;
    View& view;
};

int main() {
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif
    Model model;
    View view(model);
    Controller controller(model, view);
    controller.run();
    return 0;
}
