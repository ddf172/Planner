#include <iostream>
#include <fstream>
#include "ScheduleData.hpp"

struct InputData {
    std::vector<TimeBlock> timeblocks;
    std::vector<Class> classes;
    std::vector<Group> groups;
    std::vector<Room> rooms;
    std::vector<Teacher> teachers;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(InputData, timeblocks, classes, groups, rooms, teachers)
};

int main() {
    std::ifstream inFile("data/input_data.json");
    if (!inFile) {
        std::cerr << "Nie znaleziono pliku input_data.json" << std::endl;
        return 1;
    }

    json j;
    inFile >> j;

    InputData input = j.get<InputData>();

    std::cout << "Wczytano " << input.classes.size() << " klas(y), "
              << input.teachers.size() << " nauczyciel(i), "
              << input.rooms.size() << " sal(e)." << std::endl;

    // Przykład: wypisz wszystkie klasy
    for (const auto& cls : input.classes) {
        std::cout << "Klasa: " << cls.name << " (przedmiot: " << cls.subject << ")" << std::endl;
    }

    return 0;
}
