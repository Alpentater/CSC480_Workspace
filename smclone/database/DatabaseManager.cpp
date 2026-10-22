// Header file for the DatabaseManager class.
#include "DatabaseManager.h"

// Qt classes used for filesystem access, SQL queries/errors,
// application data paths, string lists, and SQL NULL values.
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QStringList>
#include <QVariant>

// Default constructor for the database manager.
DatabaseManager::DatabaseManager() = default;

// Destructor closes the SQLite database connection if it is still open.
DatabaseManager::~DatabaseManager(){
    if(m_database.isOpen()){
        m_database.close();
    }
}

// Initializes the database connection and prepares the application data.
// This creates the data directory, opens SQLite, enables foreign keys,
// configures a short busy timeout, creates the schema, and seeds technicians.
bool DatabaseManager::initialize() {
    // Ask Qt for a writable application-data directory.
    const QString dataDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    // Create the application-data directory if it does not already exist.
    if(!QDir().mkpath(dataDirectory)){
        setError("Could not create the application data directory: "+dataDirectory);
        return false;
    }

    // Build the path to the local SQLite database file.
    m_databasePath = dataDirectory + QDir::separator() + "scheduler.db";
    // Create a named Qt SQLite connection.
    m_database = QSqlDatabase::addDatabase("QSQLITE", "capstone_scheduler_connection");
    m_database.setDatabaseName(m_databasePath);

    // Stop initialization if the database cannot be opened.
    if(!m_database.open()){
        setError(m_database.lastError().text());
        return false;
    }

    QSqlQuery pragma(m_database);
    // Enable SQLite foreign-key enforcement so invalid customer/technician
    // relationships cannot be saved.
    if(!pragma.exec("PRAGMA foreign_keys = ON")){
        setError(pragma.lastError().text());
        return false;
    }

    // Wait briefly when SQLite is temporarily busy instead of failing instantly.
    if(!pragma.exec("PRAGMA busy_timeout = 2000")){
        setError(pragma.lastError().text());
        return false;
    }

    // Create tables/indexes and add default technicians.
    return createSchema() && seedTechnicians();
}

// Returns the most recent database error message.
QString DatabaseManager::lastError() const {return m_lastError; }
// Returns the full path to the SQLite database file.
QString DatabaseManager::databasePath() const { return m_databasePath; }\

// Creates the customers, technicians, and work_orders tables plus
// indexes used to improve common lookup and scheduling queries.
bool DatabaseManager::createSchema(){
    QSqlQuery query(m_database);

    // SQL definition for the customer account table.
    const QString customersSql = R"(
        CREATE TABLE IF NOT EXISTS customers (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            phone TEXT,
            email TEXT,
            service_address TEXT NOT NULL
        )
    )";

    // SQL definition for technicians shown on the schedule.
    const QString techniciansSql = R"(
        CREATE TABLE IF NOT EXISTS technicians (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            active INTEGER NOT NULL DEFAULT 1
        )
    )";

    // SQL definition for work orders. technician_id and scheduled_start
    // may be NULL while a work order is still Unscheduled in the Deck.
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

    // Index fields that are searched frequently to improve performance.
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

// Adds a few default technicians the first time the database is created.
bool DatabaseManager::seedTechnicians(){
    
    QSqlQuery query(m_database);
    if(!query.exec("SELECT COUNT(*) FROM technicians")) {
        setError(query.lastError().text());
        return false;
    }

    if(!query.next() || query.value(0).toInt() > 0){
        return true;
    }

    // Start a transaction so the related database changes succeed or fail together.
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

    // Commit the technician seed transaction.
    if(!m_database.commit()){
        setError(m_database.lastError().text());
        return false;
    }
    return true;
}

// Retrieves all customers ordered alphabetically by name.
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

// Retrieves one customer using its unique database ID.
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

// Inserts a new customer and stores the generated ID back into the object.
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

// Updates an existing customer record.
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

// Retrieves all active technicians for the schedule columns.
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

// Converts the current SQL query row into a WorkOrder object.
// This helper keeps the work-order lookup functions from duplicating conversion code.
WorkOrder DatabaseManager::workOrderFromQuery(const QSqlQuery &query) const {
    WorkOrder order;
    order.setId(query.value("id").toInt());
    order.setCustomerId(query.value("customer_id").toInt());

        // Unscheduled work orders do not have an assigned technician.
    if (query.value("technician_id").isNull()) {
        order.setTechnicianId(std::nullopt);
    } else {
        order.setTechnicianId(query.value("technician_id").toInt());
    }

        // Appointment times are stored as ISO-formatted text in SQLite.
    const QString start = query.value("scheduled_start").toString();
    order.setScheduledStart(start.isEmpty() ? QDateTime() : QDateTime::fromString(start, Qt::ISODate));
    order.setServiceAddress(query.value("service_address").toString());
    order.setServiceDescription(query.value("service_description").toString());
    order.setNotes(query.value("notes").toString());
    order.setStatus(WorkOrder::statusFromString(query.value("status").toString()));
    return order;
}

// Retrieves every work order, newest first.
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

// Retrieves non-cancelled work orders scheduled for a specific date.
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

// Retrieves work orders that should appear in the Schedule screen's Deck.
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

// Retrieves one work order using its unique database ID.
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

// Inserts a new work order into the database.
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
        // Bind the technician ID when scheduled; otherwise store SQL NULL.
    if (workOrder.technicianId()) {
        query.bindValue(":technician_id", *workOrder.technicianId());
    } else {
        query.bindValue(":technician_id", QVariant());
    }

        // Store a valid appointment as ISO text; otherwise store SQL NULL.
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

// Updates all editable values of an existing work order.
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

// Schedules or reschedules a work order.
// Technician, appointment time, and status are updated inside one transaction
// so a failure can be rolled back without leaving a partially updated order.
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

// Returns a scheduled work order to the Deck.
// Technician and appointment fields are cleared and status becomes Unscheduled
// as one transaction.
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

// Stores the most recent database error for the UI to display.
void DatabaseManager::setError(const QString &error) { m_lastError = error; }