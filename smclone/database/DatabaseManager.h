#pragma once

#include <QDate>
#include <QSqlDatabase>
#include <QString>
#include <vector>

#include "models/Customer.h"
#include "models/Technician.h"
#include "models/WorkOrder.h"

class DatabaseManager {
public:
	DatabaseManager();
	~DatabaseManager();

	bool initialize();
	QString lastError() const;
	QString databasePath() const;

	std::vector<Customer>customers();
	bool customerById(int customerId, Customer &customer);
	bool addCustomer(Customer &customer);
	bool updateCustomer(const Customer &customer);

	std::vector<Technician> technicians();

	std::vector<WorkOrder> allWorkOrders();
	std::vector<WorkOrder> workOrdersForDate(const QDate &date);
	std::vector<WorkOrder> unscheduledWorkOrders();
	bool workOrderById(int workOrderId, WorkOrder &workOrder);
	bool addWorkOrder(WorkOrder &workOrder);
	bool updateWorkOrder(const WorkOrder &workOrder);
	bool scheduleWorkOrder(int workOrderId, int technicianId, const QDateTime &start);
	bool unscheduleWorkOrder(int workOrderId);

private:
	bool createSchema();
	bool seedTechnicians();
	WorkOrder workOrderFromQuery(const class QSqlQuery &query) const;
	void setError(const QString &error);

	QSqlDatabase m_database;
	QString m_lastError;
	QString m_databasePath;
};