# Capstone Scheduler

A C++/Qt 6 desktop scheduling application for the CSC 480 Capstone project.

## Current features

- Qt Widgets desktop interface with a persistent left sidebar.
- Customer creation, lookup, and editing.
- Work-order creation, lookup, and editing.
- New work orders begin as **Unscheduled** and appear in the **Deck**.
- Day-based schedule with one column per technician and 30-minute time slots.
- Drag a work order from the Deck onto a technician/time slot to schedule it.
- Drag an already-scheduled work order to another technician/time slot to reschedule it.
- Drag a scheduled work order back to the Deck to unschedule it.
- Double-click schedule or Deck items to edit a work order.
- SQLite persistence with foreign keys, indexes, transactions, and a busy timeout.
- Input validation for required customer/work-order fields.
- Three default technicians are created automatically on the first run.

## Project structure

```text
CapstoneScheduler/
├── CMakeLists.txt
├── main.cpp
├── models/
│   ├── Customer.*
│   ├── Technician.*
│   ├── WorkOrder.*
│   └── Schedule.*
├── database/
│   └── DatabaseManager.*
└── ui/
    ├── MainWindow.*
    ├── CreateAccountScreen.*
    ├── ViewAccountScreen.*
    ├── CreateWorkOrderScreen.*
    ├── ViewWorkOrderScreen.*
    ├── ScheduleScreen.*
    └── DragDropWidgets.*
```

This corresponds to the capstone's three-layer design:

- **UI layer:** files under `ui/`
- **Logic layer:** files under `models/`
- **Data layer:** `DatabaseManager`

## Requirements

- Qt 6.x
- Qt Widgets module
- Qt SQL module with the QSQLITE driver
- CMake 3.21 or newer
- A C++17 compiler

The easiest setup on Windows is the Qt Online Installer with **Qt Creator** and a Desktop Qt 6 kit (MinGW or MSVC).

## Build with Qt Creator

1. Open Qt Creator.
2. Choose **File > Open File or Project**.
3. Select `CMakeLists.txt` in this folder.
4. Choose a Desktop Qt 6 kit.
5. Let CMake configure the project.
6. Click **Build** and then **Run**.

## Build from a terminal

If Qt 6 is available to CMake:

```bash
cmake -S . -B build
cmake --build build
```

Then run the generated `CapstoneScheduler` executable.

## Database location

The program automatically creates `scheduler.db` in Qt's writable application-data directory. The full database path is shown at the bottom of the left sidebar while the program is running.

The database contains:

- `customers`
- `technicians`
- `work_orders`

## First test workflow

1. Run the program. It opens to today's schedule.
2. Click **Create New Account** and make a customer.
3. Click **Create New Work Order** and select that customer.
4. Save the work order. It should appear in the **Deck**.
5. Drag the work order from the Deck onto a technician column and time slot.
6. Close and reopen the program. The customer, work order, and schedule assignment should still be present.
7. Drag the scheduled item back to the Deck to confirm unscheduling works.

## Current scope limitations

This is intentionally a capstone-sized scheduler rather than a full CRM. It does not currently include invoicing, payroll, automatic routing, caller-ID integration, remote technician devices, multi-user synchronization, or network/server deployment.

The schedule currently treats each work order as one 30-minute slot. Duration-aware appointments can be added later if needed.
