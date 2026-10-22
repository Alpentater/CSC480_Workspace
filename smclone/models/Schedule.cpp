#include "Schedule.h"

QDate Schedule::currentDate() const { return m_currentDate; }
void Schedule::setCurrentDate(const QDate &date) { m_currentDate = date; }

const std::vector<WorkOrder> &Schedule::workOrders() const { return m_workOrders; }
void Schedule::setWorkOrders(const std::vector<WorkOrder> &workOrders) { m_workOrders = workOrders; }
void Schedule::clear() { m_workOrders.clear(); }
