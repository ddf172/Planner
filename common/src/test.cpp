#include "ScheduleData.hpp"
#include <fstream>
#include <iostream>

int main() {
    Schedule schedule;

    // test data
    Lesson lesson{
        "L1", "CS", "1A", "T1", "R1",
        TimeBlock{"Monday", 8, 9}
    };
    schedule.lessons.push_back(lesson);

    Constraint c;
    c.type = "NotAfterHour";
    c.data = {
        {"classGroupId", "1A"},
        {"hour", 16}
    };
    schedule.constraints.push_back(c);

    // ckeck if the JSON file was found
    std::ifstream file("./data/schedule.json");
    if (!file) {
        std::cerr << "File not found!" << std::endl;
        return 1;
    } else {
        std::cout << "File found!" << std::endl;
    }

    try {
        // serialize the schedule to JSON
        json j = schedule;
        std::cout << "Serialized JSON: " << j.dump(4) << std::endl;

        // write the JSON to a file
        std::ofstream outFile("./data/schedule.json");
        if (outFile.is_open()) {
            outFile << j.dump(4);
            outFile.close();
            std::cout << "JSON written to file." << std::endl;
        } else {
            std::cerr << "Error opening file for writing." << std::endl;
        }

    } catch (const json::exception& e) {
        std::cerr << "JSON error: " << e.what() << std::endl;
    }

    // print the current directory using a system call to bash
    std::system("pwd");


    return 0;
}
