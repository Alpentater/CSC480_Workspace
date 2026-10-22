#include "DatabaseManager.h"

#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QStringList>
#include <QVariant>

Database::DatabaseManager() = default;

Database::~DatabaseManager(){
	if(m_database.isOpen()){
		m_database.close();
	}
}

bool DatabaseManager::initialize() {
	const QString dataDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

	if(!QDir().mkpath(dataDirectory)){
		setError("Could not create the application data directory: "+dataDirectory);
		return false;
	}

	m_databasePath = dataDirectory + QDir::seperator() + "scheduler.db";
	m_database = QSqlDatabase::addDatabase("QSQLITE", "capstone_scheduler_connection");
	m_database.setDatabaseName(m_databasePath);

	if(!m_database.open()){
		setError(m_database.lastError().text());
		return false;
	}

	QSqlQuery pragma(m_database);
	if(!pragma.exec("PRAGMA foreign_keys = ON")){
		setError(pragma.lastError().text());
		return false;
	}

	if(!pragma.exec("PRAGMA busy_timeout = 2000")){
		setError(pragma.lastError().text());
		return false;
	}

	return createSchema() && seedTechnicians();
}

QString DatabaseManager::lastError() const {return m_lastError; }
QString DatabaseManager::databasePath() const { return m_databasePath; }\

bool DatabaseManager::createSchema(){
	QSqlQuery query(m_database);

	const QString customersSql = R"(
		CREATE TABLE IF NOT EXISTS customers (
			id INTEGER PRIMARY KEY AUTOINCREMENT,
			name TEXT NOT NULL,
			phone TEXT,
			email TEXT,
			service_address TEXT NOT NULL
		)
	)";

	const QString techniciansSql = R"(
		CREATE TABLE IF NOT EXISTS technicians (
			id INTEGER PRIMARY KEY AUTOINCREMENT,
			name TEXT NOT NULL,
			active INTEGER NOT NULL DEFAULT 1
		)
	)";

	const QString workOrdersSql = R"(
		CREATE TABLE IF NOT EXISTS work_orders(
			id INTEGER PRIMARY KEY AUTOINCREMENT,
			customer_id INTEGER NOT NULL,
			technician_id INTEGER,
			scheduled_start TEXT,
			service_address TEXT NOT NULL,
			service_description TEXT NOT NULL,
			notes TEXT,
			status TEXT NOT NULL DEFAULT 'Unscheduled',
			FOREIGN KEY(customer_id) REFERENCES customers(id),
			FOREIGN KEY(technician_id) REFERENCES technicians(id)
		)
	)";

	if(!query.exec(customersSql) || !query.exec(techniciansSql) || !query.exec(workOrdersSql)) {
		setError(query.lastError().text());
		return false;
	}

	const QStringList indexes = {
        "CREATE INDEX IF NOT EXISTS idx_work_orders_customer ON work_orders(customer_id)",
        "CREATE INDEX IF NOT EXISTS idx_work_orders_technician ON work_orders(technician_id)",
        "CREATE INDEX IF NOT EXISTS idx_work_orders_start ON work_orders(scheduled_start)"
	};

	for (const QString &sql : indexes){
		if(!query.exec(sql)){
			setError(query.lastError().text());
			return false;
		}
	}

	return true;
}

bool DatabaseManager::seedTechnicians(){
	
	QSqlQuery query(m_database);
	if(!query.exec("SELECT COUNT(*) FROM technicians")) {
		setError(query.lastError().text());
		return false;
	}

	if(!query.next() || query.value(0).toInt() > 0){
		return true;
	}

	if(!m_database.transaction()){
		setError(m_database.lastError().text());
		return false;
	}

	QSqlQuery insert(m_database);
	insert.prepare("INSERT INTO technicians(name, active) VALUES(:name, 1)");

	const QStringList names = {"Technician 1", "Technician 2", "Technician 3"};
	for(const QString &name : names){
		insert.bindValue(":name", name);
		if(!insert.exec()){
			m_database.rollback();
			setError(insert.lastError().text());
			return false;
		}
	}

	if(!m_database.commit()){
		setError(m_database.lastError().text());
		return false;
	}
	return true;
}

std::vector<Customer> DatabaseManager::customers(){
	
	std::vector<Customer> result;
	QSqlQuery query(m_database);

	if(!query.exec("SELECT id, name, phone, email, service_address FROM customers ORDER BY name")){
		setError(query.lastError().text());
		return result;
	}

	while(query.next()){
		result.emplace_back(query.value(0).toInt(), query.value(1).toString(), query.value(2).toString(), query.value(3).toString(), query.value(4).toString());
	}
	return result;
}

bool DatabaseManager::customerById(int customerId, Customer &customer){

    QSqlQuery query(m_database);
    query.prepare("SELECT id, name, phone, email, service_address FROM customers WHERE id = :id");
    query.bindValue(":id", customerId);

    if (!query.exec()) {
        setError(query.lastError().text());
        return false;
    }

    if (!query.next()) {
        setError("Customer not found.");
        return false;
    }

    customer = Customer(query.value(0).toInt(),
                        query.value(1).toString(),
                        query.value(2).toString(),
                        query.value(3).toString(),
                        query.value(4).toString());
    return true;
}

bool DatabaseManager::addCustomer(Customer &customer) {
    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO customers(name, phone, email, service_address)
        VALUES(:name, :phone, :email, :service_address)
    )");

    query.bindValue(":name", customer.name());
    query.bindValue(":phone", customer.phone());
    query.bindValue(":email", customer.email());
    query.bindValue(":service_address", customer.serviceAddress());

    if (!query.exec()) {
        setError(query.lastError().text());
        return false;
    }

    customer.setId(query.lastInsertId().toInt());
    return true;
}

bool DatabaseManager::updateCustomer(const Customer &customer){
    QSqlQuery query(m_database);
    query.prepare(R"(
        UPDATE customers
        SET name = :name,
            phone = :phone,
            email = :email,
            service_address = :service_address
        WHERE id = :id
    )");
    query.bindValue(":name", customer.name());
    query.bindValue(":phone", customer.phone());
    query.bindValue(":email", customer.email());
    query.bindValue(":service_address", customer.serviceAddress());
    query.bindValue(":id", customer.id());

    if (!query.exec()) {
        setError(query.lastError().text());
        return false;
    }
    return true;
}

std::vector<Technician> DatabaseManager::technicians() {
    std::vector<Technician> result;
    QSqlQuery query(m_database);

    if (!query.exec("SELECT id, name, active FROM technicians WHERE active = 1 ORDER BY id")) {
        setError(query.lastError().text());
        return result;
    }

    while (query.next()) {
        result.emplace_back(query.value(0).toInt(), query.value(1).toString(), query.value(2).toBool());
    }

    return result;
}

WorkOrder DatabaseManager::workOrderFromQuery(const QSqlQuery &query) const {
    WorkOrder order;
    order.setId(query.value("id").toInt());
    order.setCustomerId(query.value("customer_id").toInt());

    if (query.value("technician_id").isNull()) {
        order.setTechnicianId(std::nullopt);
    } else {
        order.setTechnicianId(query.value("technician_id").toInt());
    }

    const QString start = query.value("scheduled_start").toString();
    order.setScheduledStart(start.isEmpty() ? QDateTime() : QDateTime::fromString(start, Qt::ISODate));
    order.setServiceAddress(query.value("service_address").toString());
    order.setServiceDescription(query.value("service_description").toString());
    order.setNotes(query.value("notes").toString());
    order.setStatus(WorkOrder::statusFromString(query.value("status").toString()));
    return order;
}

std::vector<WorkOrder> DatabaseManager::allWorkOrders() {
    std::vector<WorkOrder> result;
    QSqlQuery query(m_database);

    if (!query.exec("SELECT * FROM work_orders ORDER BY id DESC")) {
        setError(query.lastError().text());
        return result;
    }

    while (query.next()) {
        result.push_back(workOrderFromQuery(query));
    }
    return result;
}

std::vector<WorkOrder> DatabaseManager::workOrdersForDate(const QDate &date) {
    std::vector<WorkOrder> result;
    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT * FROM work_orders
        WHERE scheduled_start IS NOT NULL
          AND date(scheduled_start) = :date
          AND status <> 'Cancelled'
        ORDER BY scheduled_start
    )");
    query.bindValue(":date", date.toString(Qt::ISODate));

    if (!query.exec()) {
        setError(query.lastError().text());
        return result;
    }

    while (query.next()) {
        result.push_back(workOrderFromQuery(query));
    }
    return result;
}

std::vector<WorkOrder> DatabaseManager::unscheduledWorkOrders() {
    std::vector<WorkOrder> result;
    QSqlQuery query(m_database);

    if (!query.exec(R"(
        SELECT * FROM work_orders
        WHERE status = 'Unscheduled'
        ORDER BY id DESC
    )")) {
        setError(query.lastError().text());
        return result;
    }

    while (query.next()) {
        result.push_back(workOrderFromQuery(query));
    }
    return result;
}

bool DatabaseManager::workOrderById(int workOrderId, WorkOrder &workOrder){
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM work_orders WHERE id = :id");
    query.bindValue(":id", workOrderId);

    if (!query.exec()) {
        setError(query.lastError().text());
        return false;
    }
    if (!query.next()) {
        setError("Work order not found.");
        return false;
    }

    workOrder = workOrderFromQuery(query);
    return true;
}

bool DatabaseManager::addWorkOrder(WorkOrder &workOrder) {
    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO work_orders(
            customer_id, technician_id, scheduled_start,
            service_address, service_description, notes, status)
        VALUES(
            :customer_id, :technician_id, :scheduled_start,
            :service_address, :service_description, :notes, :status)
    )");

    query.bindValue(":customer_id", workOrder.customerId());
    if (workOrder.technicianId()) {
        query.bindValue(":technician_id", *workOrder.technicianId());
    } else {
        query.bindValue(":technician_id", QVariant());
    }

    if (workOrder.scheduledStart().isValid()) {
        query.bindValue(":scheduled_start", workOrder.scheduledStart().toString(Qt::ISODate));
    } else {
        query.bindValue(":scheduled_start", QVariant());
    }

    query.bindValue(":service_address", workOrder.serviceAddress());
    query.bindValue(":service_description", workOrder.serviceDescription());
    query.bindValue(":notes", workOrder.notes());
    query.bindValue(":status", WorkOrder::statusToString(workOrder.status()));

    if (!query.exec()) {
        setError(query.lastError().text());
        return false;
    }

    workOrder.setId(query.lastInsertId().toInt());
    return true;
}

bool DatabaseManager::updateWorkOrder(const WorkOrder &workOrder) {
    QSqlQuery query(m_database);
    query.prepare(R"(
        UPDATE work_orders
        SET customer_id = :customer_id,
            technician_id = :technician_id,
            scheduled_start = :scheduled_start,
            service_address = :service_address,
            service_description = :service_description,
            notes = :notes,
            status = :status
        WHERE id = :id
    )");

    query.bindValue(":customer_id", workOrder.customerId());
    if (workOrder.technicianId()) {
        query.bindValue(":technician_id", *workOrder.technicianId());
    } else {
        query.bindValue(":technician_id", QVariant());
    }

    if (workOrder.scheduledStart().isValid()) {
        query.bindValue(":scheduled_start", workOrder.scheduledStart().toString(Qt::ISODate));
    } else {
        query.bindValue(":scheduled_start", QVariant());
    }

    query.bindValue(":service_address", workOrder.serviceAddress());
    query.bindValue(":service_description", workOrder.serviceDescription());
    query.bindValue(":notes", workOrder.notes());
    query.bindValue(":status", WorkOrder::statusToString(workOrder.status()));
    query.bindValue(":id", workOrder.id());

    if (!query.exec()) {
        setError(query.lastError().text());
        return false;
    }
    return true;
}

bool DatabaseManager::scheduleWorkOrder(int workOrderId, int technicianId, const QDateTime &start) {
    if (!m_database.transaction()) {
        setError(m_database.lastError().text());
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(R"(
        UPDATE work_orders
        SET technician_id = :technician_id,
            scheduled_start = :scheduled_start,
            status = 'Scheduled'
        WHERE id = :id
    )");
    query.bindValue(":technician_id", technicianId);
    query.bindValue(":scheduled_start", start.toString(Qt::ISODate));
    query.bindValue(":id", workOrderId);

    if (!query.exec()) {
        m_database.rollback();
        setError(query.lastError().text());
        return false;
    }

    if (!m_database.commit()) {
        m_database.rollback();
        setError(m_database.lastError().text());
        return false;
    }

    return true;
}

bool DatabaseManager::unscheduleWorkOrder(int workOrderId) {
    if (!m_database.transaction()) {
        setError(m_database.lastError().text());
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(R"(
        UPDATE work_orders
        SET technician_id = NULL,
            scheduled_start = NULL,
            status = 'Unscheduled'
        WHERE id = :id
    )");
    query.bindValue(":id", workOrderId);

    if (!query.exec()) {
        m_database.rollback();
        setError(query.lastError().text());
        return false;
    }

    if (!m_database.commit()) {
        m_database.rollback();
        setError(m_database.lastError().text());
        return false;
    }

    return true;
}

void DatabaseManager::setError(const QString &error) { m_lastError = error; }
