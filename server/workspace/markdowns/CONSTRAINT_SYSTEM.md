# School Schedule Constraint System

This document defines the comprehensive constraint system for the school schedule generation application. Each constraint type has a specific JSON data structure that defines the entities involved and their relationships.

## Core Constraint Structure

```cpp
struct Constraint {
    std::string importance;     // "Critical", "Important", "Optional"
    std::string description;    // Human-readable description
    ConstraintType type;        // Enumerated constraint type
    json data;                  // Type-specific constraint data
};
```

## Constraint Categories and Types

### 1. Time-Based Constraints

#### TeacherUnavailable
**Purpose**: Teacher cannot be scheduled during specific time periods
**Subject**: Teacher ID
**Relation**: NOT_AVAILABLE_AT
**Context**: Time blocks/ranges

```json
{
    "teacherId": "T1",
    "timeBlocks": ["TB1", "TB2"],
    "reason": "Meeting with parents"
}
```

Alternative format for recurring unavailability:
```json
{
    "teacherId": "T1",
    "days": ["Monday", "Friday"],
    "timeRanges": [{"start": 800, "end": 1000}],
    "recurring": true
}
```

#### TeacherPreferred
**Purpose**: Teacher prefers to teach during specific times
**Subject**: Teacher ID
**Relation**: PREFERS
**Context**: Time blocks/ranges with priority

```json
{
    "teacherId": "T1",
    "timeBlocks": ["TB3", "TB4"],
    "priority": 8,
    "reason": "Peak performance hours"
}
```

#### GroupUnavailable
**Purpose**: Student group is not available during specific times
**Subject**: Group ID
**Relation**: NOT_AVAILABLE_AT
**Context**: Time blocks (e.g., other activities)

```json
{
    "groupId": "G1",
    "timeBlocks": ["TB5"],
    "reason": "Physical education"
}
```

### 2. Resource Constraints

#### RequiredRoomFeature
**Purpose**: Class requires specific room features
**Subject**: Subject/Class ID
**Relation**: REQUIRES
**Context**: Room features with logical operators

```json
{
    "subjectId": "CS_LAB",
    "features": ["computers", "projector"],
    "operator": "AND",
    "minCapacity": 20
}
```

#### PreferredRoom
**Purpose**: Class prefers specific rooms
**Subject**: Subject/Class ID
**Relation**: PREFERS
**Context**: Room IDs with priority

```json
{
    "subjectId": "CHEMISTRY",
    "roomIds": ["R_LAB1", "R_LAB2"],
    "priority": 9,
    "reason": "Specialized equipment"
}
```

#### ForbiddenRoom
**Purpose**: Class cannot use specific rooms
**Subject**: Subject/Class ID
**Relation**: CANNOT_USE
**Context**: Room IDs

```json
{
    "subjectId": "PHYSICAL_ED",
    "roomIds": ["R_COMPUTER_LAB"],
    "reason": "Incompatible activity type"
}
```

#### MinimumRoomCapacity
**Purpose**: Room must accommodate minimum number of students
**Subject**: Group/Class size
**Relation**: MUST_BE_GREATER_THAN_OR_EQUAL
**Context**: Room capacity

```json
{
    "groupId": "G1",
    "minCapacity": 25,
    "includeBuffer": true,
    "bufferPercentage": 10
}
```

### 3. Teacher Constraints

#### MaxTeachingHours
**Purpose**: Limit teacher's working hours
**Subject**: Teacher ID
**Relation**: MUST_NOT_EXCEED
**Context**: Hours per period

```json
{
    "teacherId": "T1",
    "maxHours": 6,
    "period": "day",
    "includeBreaks": false
}
```

#### MinBreakBetweenClasses
**Purpose**: Ensure minimum break time between consecutive classes
**Subject**: Teacher ID
**Relation**: MUST_HAVE_GAP_OF
**Context**: Minimum break duration

```json
{
    "teacherId": "T1",
    "minBreakMinutes": 15,
    "applyToConsecutive": true,
    "allowLunchBreak": true
}
```

#### SameTeacherForSubject
**Purpose**: Ensure consistency in teaching assignments
**Subject**: Subject ID + Group ID
**Relation**: MUST_BE_TAUGHT_BY_SAME
**Context**: Teacher assignment scope

```json
{
    "subjectId": "MATHEMATICS",
    "groupId": "G1",
    "scope": "semester",
    "allowSubstitute": false
}
```

#### TeacherSubjectMatch
**Purpose**: Verify teacher qualification for subject
**Subject**: Teacher ID
**Relation**: IS_QUALIFIED_FOR
**Context**: Subject IDs

```json
{
    "teacherId": "T1",
    "allowedSubjects": ["CS", "MATH", "PHYSICS"],
    "restrictive": true
}
```

### 4. Group Constraints

#### MaxClassesPerDay
**Purpose**: Limit daily class load for students
**Subject**: Group ID
**Relation**: MUST_NOT_EXCEED
**Context**: Daily class count

```json
{
    "groupId": "G1",
    "maxClasses": 6,
    "includeBreaks": false,
    "countDoublePeriods": true
}
```

#### GroupSplit
**Purpose**: Allow group division for specific subjects
**Subject**: Group ID
**Relation**: CAN_BE_DIVIDED_INTO
**Context**: Subgroup specifications

```json
{
    "groupId": "G1",
    "maxSubgroupSize": 15,
    "subjectIds": ["CS_LAB", "LANGUAGE_LAB"],
    "parallelTeaching": true
}
```

#### GroupMerge
**Purpose**: Allow multiple groups to attend together
**Subject**: Multiple Group IDs
**Relation**: CAN_BE_COMBINED_FOR
**Context**: Subject and capacity constraints

```json
{
    "groupIds": ["G1", "G2"],
    "subjectIds": ["ASSEMBLY", "LECTURE"],
    "maxCombinedSize": 60,
    "requiresSameLevel": true
}
```

### 5. Schedule Pattern Constraints

#### ConsecutiveClasses
**Purpose**: Enforce or prefer consecutive scheduling
**Subject**: Subject ID + Group ID
**Relation**: SHOULD_BE_SCHEDULED_CONSECUTIVELY
**Context**: Consecutive count and preference level

```json
{
    "subjectId": "MATHEMATICS",
    "groupId": "G1",
    "consecutiveCount": 2,
    "preference": "required",
    "allowBreakBetween": false
}
```

#### AvoidConsecutive
**Purpose**: Prevent consecutive scheduling of heavy subjects
**Subject**: Subject IDs
**Relation**: SHOULD_NOT_BE_CONSECUTIVE
**Context**: Subject combinations to avoid

```json
{
    "subjectIds": ["PHYSICS", "CHEMISTRY", "MATHEMATICS"],
    "groupId": "G1",
    "maxConsecutive": 1,
    "applyToSameDay": true
}
```

#### SpreadAcrossWeek
**Purpose**: Distribute subject instances across the week
**Subject**: Subject ID + Group ID
**Relation**: SHOULD_BE_DISTRIBUTED_ACROSS
**Context**: Distribution pattern

```json
{
    "subjectId": "ENGLISH",
    "groupId": "G1",
    "instancesPerWeek": 3,
    "minDaysBetween": 1,
    "preferredDays": ["Monday", "Wednesday", "Friday"]
}
```

### 6. Dependency Constraints

#### ClassBefore
**Purpose**: One class must occur before another
**Subject**: Two Subject/Class IDs
**Relation**: MUST_OCCUR_BEFORE
**Context**: Timing and dependency scope

```json
{
    "beforeSubject": "THEORY_PHYSICS",
    "afterSubject": "LAB_PHYSICS",
    "groupId": "G1",
    "sameDay": true,
    "maxGapMinutes": 120,
    "allowSameTimeSlot": false
}
```

#### SameTimeSlot
**Purpose**: Classes must be scheduled simultaneously
**Subject**: Multiple Subject/Class IDs
**Relation**: MUST_OCCUR_AT_SAME_TIME
**Context**: Synchronization requirements

```json
{
    "subjectIds": ["GROUP_A_LANGUAGE", "GROUP_B_LANGUAGE"],
    "reason": "Shared teacher rotation",
    "strictTiming": true
}
```

## Constraint Validation Rules

### Importance Levels
- **Critical**: Must be satisfied, schedule is invalid if violated
- **Important**: Should be satisfied, violations penalize the solution
- **Optional**: Nice to have, minimal impact on solution quality

### Conflict Resolution
When constraints conflict, resolution follows this hierarchy:
1. Critical constraints always take precedence
2. Important constraints are weighted by domain expertise
3. Optional constraints are satisfied when possible

### Data Validation Schema
Each constraint type has a JSON schema for validation:

```json
{
    "TeacherUnavailable": {
        "required": ["teacherId"],
        "oneOf": [
            {"required": ["timeBlocks"]},
            {"required": ["days", "timeRanges"]}
        ],
        "properties": {
            "teacherId": {"type": "string"},
            "timeBlocks": {"type": "array", "items": {"type": "string"}},
            "days": {"type": "array", "items": {"type": "string"}},
            "timeRanges": {
                "type": "array",
                "items": {
                    "type": "object",
                    "required": ["start", "end"],
                    "properties": {
                        "start": {"type": "integer", "minimum": 0, "maximum": 2359},
                        "end": {"type": "integer", "minimum": 0, "maximum": 2359}
                    }
                }
            }
        }
    }
}
```

## Usage Examples

### Example 1: Computer Science Lab Requirements
```json
{
    "importance": "Critical",
    "description": "CS Lab requires computers and can only fit 20 students",
    "type": "RequiredRoomFeature",
    "data": {
        "subjectId": "CS_LAB",
        "features": ["computers", "internet"],
        "operator": "AND",
        "minCapacity": 20,
        "maxCapacity": 20
    }
}
```

### Example 2: Teacher Schedule Preference
```json
{
    "importance": "Important",
    "description": "Mrs. Smith prefers morning classes due to commute",
    "type": "TeacherPreferred",
    "data": {
        "teacherId": "T_SMITH",
        "timeRanges": [{"start": 800, "end": 1200}],
        "days": ["Monday", "Tuesday", "Wednesday", "Thursday", "Friday"],
        "priority": 8
    }
}
```

### Example 3: Complex Dependency
```json
{
    "importance": "Critical",
    "description": "Physics theory must precede physics lab on same day",
    "type": "ClassBefore",
    "data": {
        "beforeSubject": "PHYSICS_THEORY",
        "afterSubject": "PHYSICS_LAB",
        "groupId": "G_SCIENCE_A",
        "sameDay": true,
        "maxGapMinutes": 60,
        "minGapMinutes": 15
    }
}
```

This constraint system provides a flexible, extensible framework for defining complex scheduling requirements while maintaining clear relationships between entities and their constraints.
