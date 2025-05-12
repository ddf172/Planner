#pragma once

#include <string>
#include <vector>
#include <set>
#include "json.hpp"

using json = nlohmann::json;

// --- TimeBlock ---
struct TimeBlock {
    std::string day;     // "Monday", "Tuesday", ...
    int startHour;       // 8
    int endHour;         // 9

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(TimeBlock, day, startHour, endHour)
};

// --- Room ---
struct Room {
    std::string id;
    std::string name;
    std::set<std::string> features; // e.g. "computers", "projector"

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Room, id, name, features)
};

// --- Teacher ---
struct Teacher {
    std::string id;
    std::string name;
    std::set<std::string> subjects;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Teacher, id, name, subjects)
};

// --- ClassGroup ---
struct ClassGroup {
    std::string id;
    std::string name;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ClassGroup, id, name)
};

// --- Subject ---
struct Subject {
    std::string name;
    std::set<std::string> requiredRoomFeatures;
    std::string requiredTeacherSubject;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Subject, name, requiredRoomFeatures, requiredTeacherSubject)
};

// --- Lesson ---
struct Lesson {
    std::string id;
    std::string subject;
    std::string classGroupId;
    std::string teacherId;
    std::string roomId;
    TimeBlock time;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Lesson, id, subject, classGroupId, teacherId, roomId, time)
};

// --- Constraint (generic) ---
struct Constraint {
    std::string type;     // "NotAfterHour", "MustHaveTeacher", etc.
    json data;            // Constraint-specific fields

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Constraint, type, data)
};

// --- Optional: Schedule (container for all lessons) ---
struct Schedule {
    std::vector<Lesson> lessons;
    std::vector<Constraint> constraints;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Schedule, lessons, constraints)
};

