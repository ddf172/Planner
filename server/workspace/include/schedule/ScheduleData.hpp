#pragma once

#include <string>
#include <vector>
#include <set>
#include <optional>
#include "json.hpp"

using json = nlohmann::json;


// --- TimeBlock ---
struct TimeBlock {
    std::string id;
    std::string day;        // e.g. "Monday"
    int start;              // e.g. 800 (8:00)
    int end;                // e.g. 945 (9:45)
    int duration;           // minutes

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(TimeBlock, id, day, start, end, duration)
};

// --- Subject ---
struct Subject {
    std::string id;
    std::string name;
    float hoursPerWeek; // np. 3.0
    int difficultyLevel; // np. 1-5

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Subject, id, name, hoursPerWeek, difficultyLevel)
};

// --- Group ---
struct Group {
    std::string id;
    std::string name;
    int size;
    std::string parentGroupId; // Null if no parent group

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Group, id, name, size, parentGroupId)
};

// --- Room ---
struct Room {
    std::string id;
    std::string name;
    int capacity;
    std::set<std::string> features;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Room, id, name, capacity, features)
};

// --- Teacher ---
struct Teacher {
    std::string id;
    std::string name;
    std::vector<std::string> subjects;
    std::vector<int> availableTimeBlocks; // ids of TimeBlocks

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Teacher, id, name, subjects, availableTimeBlocks)
};

// --- Event (Class) ---
struct Event {
    std::string id;
    std::string subjectId;
    std::string teacherId;
    std::string groupId;
    std::string roomId;
    std::string timeBlockId;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Event, id, subjectId, teacherId, groupId, roomId, timeBlockId)
};

// --- Schedule (output) ---
struct Schedule {
    std::vector<Event> events;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Schedule, events)
};


// --- Constraint ---
struct Constraint {
    std::string importance; // "Critical", "Important", "Optional"
    std::string description; // Human-readable description
    ConstraintType type;    // Type of constraint
    json data;             // Type-specific constraint data

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Constraint, importance, description, type, data)
};

// --- Constraint Type ---
enum class ConstraintType {
    // Time-based constraints
    TeacherUnavailable,     // Teacher not available at specific times
    TeacherPreferred,       // Teacher prefers specific times
    GroupUnavailable,       // Group not available at specific times
    
    // Resource constraints
    RequiredRoomFeature,    // Class requires specific room features
    PreferredRoom,          // Class prefers specific room
    ForbiddenRoom,          // Class cannot use specific room
    MinimumRoomCapacity,    // Room must have minimum capacity
    
    // Teacher constraints
    MaxTeachingHours,       // Teacher max hours per day/week
    MinBreakBetweenClasses, // Minimum break time between classes
    SameTeacherForSubject,  // Same teacher for all instances of subject
    TeacherSubjectMatch,    // Teacher must be qualified for subject
    
    // Group constraints
    MaxClassesPerDay,       // Group max classes per day
    GroupSplit,             // Group can be split into subgroups
    GroupMerge,             // Multiple groups can attend together
    
    // Schedule patterns
    ConsecutiveClasses,     // Classes should be consecutive
    AvoidConsecutive,       // Classes should not be consecutive
    SameDayClasses,         // Multiple instances on same day
    SpreadAcrossWeek,       // Distribute classes across week
    
    // Dependencies
    ClassBefore,            // Class A must be before class B
    ClassAfter,             // Class A must be after class B
    SameTimeSlot,           // Classes must be at same time
    
    // Custom constraints
    Custom                  // For application-specific constraints
};

// --- Constraint Data Structures ---
// Each constraint type uses specific JSON structure in the 'data' field

/*
CONSTRAINT DATA STRUCTURE EXAMPLES:

1. TeacherUnavailable:
{
    "teacherId": "T1",
    "timeBlocks": ["TB1", "TB2"] OR
    "days": ["Monday", "Friday"],
    "timeRanges": [{"start": 800, "end": 1000}]
}

2. RequiredRoomFeature:
{
    "features": ["computers", "projector"],
    "operator": "AND" | "OR"  // All features required vs any feature
}

3. MaxTeachingHours:
{
    "teacherId": "T1",
    "maxHours": 6,
    "period": "day" | "week"
}

4. MinBreakBetweenClasses:
{
    "teacherId": "T1",
    "minBreakMinutes": 15,
    "applyToConsecutive": true
}

5. GroupSplit:
{
    "groupId": "G1",
    "maxSubgroupSize": 15,
    "subjectIds": ["CS", "LAB"]  // Optional: only for specific subjects
}

6. ConsecutiveClasses:
{
    "subjectId": "MATH",
    "groupId": "G1",
    "consecutiveCount": 2,
    "preference": "required" | "preferred" | "avoided"
}

7. ClassBefore:
{
    "beforeSubject": "THEORY",
    "afterSubject": "LAB",
    "groupId": "G1",
    "sameDay": true,
    "maxGapMinutes": 120
}

8. PreferredRoom:
{
    "roomIds": ["R1", "R2"],
    "priority": 1-10,
    "subjects": ["CS"]  // Optional: only for specific subjects
}

9. MaxClassesPerDay:
{
    "groupId": "G1",
    "maxClasses": 6,
    "includeBreaks": false
}

10. Custom:
{
    "type": "application_specific_type",
    "parameters": { ... }  // Application-specific data
}
*/
