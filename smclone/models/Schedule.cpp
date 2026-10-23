//Header file for the schedule class.
#include "Schedule.h"

//Getting and setting the current date on the schedule.
QDate Schedule::currentDate() const { return m_currentDate; }
void Schedule::setCurrentDate(const QDate &date) { m_currentDate = date; }

//Returns a constant reference to the collection of work order currently loaded for the selected schedule date.
//Returning a reference avoids making an unnecessary copy of the entire std::vector<WorkOrder>.
const std::vector<WorkOrder> &Schedule::workOrders() const { return m_workOrders; }

//Replaces the current collection of work orders.
//This is typically called after the database loads all work orders for the currently selected date.
void Schedule::setWorkOrders(const std::vector<WorkOrder> &workOrders) { m_workOrders = workOrders; }

//Removes all work orders currently stored in the Schedule object.
//This only clears the in-memory vector. It does not delete any work orders from the SQLite database.
void Schedule::clear() { m_workOrders.clear(); }
