#pragma once

#include <QDate>
#include <vector>

#include "WorkOrder.h"

class Schedule {
public:
	QDate currentDate() const;
	void setCurrentDate(const QDate &date);

	const std::vector<WorkOrder> &workOrders() const;
	void setWorkOrders(const std::vector<WorkOrder> &workOrders);
	void clear();

private:
	QDate m_currentDate = QDate::currentDate();
	std::vector<WorkOrder> m_workOrders;
};