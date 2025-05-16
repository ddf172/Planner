#pragma once

#include <string>
#include <vector>
#include <set>
#include <optional>
#include "json.hpp"

using json = nlohmann::json;

// --- Constraint ---
struct Constraint {
    std::string type;       // np. "Unavailable", "AvoidGroup", "RequiredRoomFeature", ...
    json data;              // elastyczne dane związane z typem

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Constraint, type, data)
};

// --- TimeBlock ---
struct TimeBlock {
    std::string id;
    std::string day;        // np. "Monday"
    int start;              // np. 800 (8:00)
    int end;                // np. 945 (9:45)
    int duration;           // minuty

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(TimeBlock, id, day, start, end, duration)
};

// --- Class ---
struct Class {
    std::string id;
    std::string name;
    std::string subject;
    int difficulty;
    std::vector<Constraint> constraints;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Class, id, name, subject, difficulty, constraints)
};

// --- Group ---
struct Group {
    std::string id;
    std::string name;
    std::string parentGroupId; // Null jeśli nie jest podgrupą
    std::vector<Constraint> constraints;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Group, id, name, parentGroupId, constraints)
};

// --- Room ---
struct Room {
    std::string id;
    std::string name;
    std::set<std::string> features;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Room, id, name, features)
};

// --- Teacher ---
struct Teacher {
    std::string id;
    std::string name;
    std::string subject;
    std::vector<Constraint> constraints;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Teacher, id, name, subject, constraints)
};

// --- Event (Planowane zajęcia) ---
struct Event {
    std::string id;
    std::string classId;
    std::string teacherId;
    std::string groupId;
    std::string roomId;
    std::string timeBlockId;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Event, id, classId, teacherId, groupId, roomId, timeBlockId)
};

// --- Schedule (zawiera wynikowy plan) ---
struct Schedule {
    std::vector<Event> events;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Schedule, events)
};
