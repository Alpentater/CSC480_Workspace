//Includingt the header file for the work order class.
#include "WorkOrder.h"

//Getter and setter for the work order id.
int WorkOrder::id() const { return m_id; }
void WorkOrder::setId(int id) { m_id = id; }

//Getter and setter for the customer id.
int WorkOrder::customerId() const { return m_customerId; }
void WorkOrder::setCustomerId(int customerId) { m_customerId = customerId; }

//Getter and setter for the technician id.
std::optional<int> WorkOrder::technicianId() const { return m_technicianId; }
void WorkOrder::setTechnicianId(std::optional<int> technicianId) { m_technicianId = technicianId; }

//Getter and setter for the job's start time on the schedule.
QDateTime WorkOrder::scheduledStart() const { return m_scheduledStart; }
void WorkOrder::setScheduledStart(const QDateTime &scheduledStart) { m_scheduledStart = scheduledStart; }

//Getter and setter for the work orders service address.
QString WorkOrder::serviceAddress() const { return m_serviceAddress; }
void WorkOrder::setServiceAddress(const QString &serviceAddress) { m_serviceAddress = serviceAddress; }

//Getter and setter for the service description field on the work order.
QString WorkOrder::serviceDescription() const { return m_serviceDescription; }
void WorkOrder::setServiceDescription(const QString &serviceDescription) { m_serviceDescription = serviceDescription; }

//Getter and setter for the notes of a work order.
QString WorkOrder::notes() const { return m_notes; }
void WorkOrder::setNotes(const QString &notes) { m_notes = notes; }

//Getter and setter for the status of a work order.
WorkOrder::Status WorkOrder::status() const { return m_status; }
void WorkOrder::setStatus(Status status) { m_status = status; }

//Status of a work order. grabbing them as strings to print with.
QString WorkOrder::statusToString(Status status) {
    switch (status) {
    case Status::Unscheduled: return "Unscheduled";
    case Status::Scheduled: return "Scheduled";
    case Status::InProgress: return "In Progress";
    case Status::Completed: return "Completed";
    case Status::Cancelled: return "Cancelled";
    }
    return "Unscheduled"; //Default work order status.
}

WorkOrder::Status WorkOrder::statusFromString(const QString &status) {
    if (status == "Scheduled") return Status::Scheduled;
    if (status == "In Progress") return Status::InProgress;
    if (status == "Completed") return Status::Completed;
    if (status == "Cancelled") return Status::Cancelled;
    return Status::Unscheduled;
}
