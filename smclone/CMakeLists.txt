cmake_minimum_required(VERSION 3.21)

project(CapstoneScheduler VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

qt_add_executable(CapstoneScheduler
    main.cpp

    models/Customer.h
    models/Customer.cpp
    models/Technician.h
    models/Technician.cpp
    models/WorkOrder.h
    models/WorkOrder.cpp
    models/Schedule.h
    models/Schedule.cpp

    database/DatabaseManager.h
    database/DatabaseManager.cpp

    ui/MainWindow.h
    ui/MainWindow.cpp
    ui/CreateAccountScreen.h
    ui/CreateAccountScreen.cpp
    ui/ViewAccountScreen.h
    ui/ViewAccountScreen.cpp
    ui/CreateWorkOrderScreen.h
    ui/CreateWorkOrderScreen.cpp
    ui/ViewWorkOrderScreen.h
    ui/ViewWorkOrderScreen.cpp
    ui/ScheduleScreen.h
    ui/ScheduleScreen.cpp
    ui/DragDropWidgets.h
    ui/DragDropWidgets.cpp
)

target_link_libraries(CapstoneScheduler PRIVATE
    Qt6::Widgets
    Qt6::Sql
)

target_include_directories(CapstoneScheduler PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
