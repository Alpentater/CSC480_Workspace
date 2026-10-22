#include "WorkOrder.h"

int WorkOrder::id() const { return m_id; }
void WorkOrder::setId(int id) { m_id = id; }

int WorkOrder::customerId() const { return m_customerId; }
void WorkOrder::setCustomerId(int customerId) { m_customerId = customerId; }

std::optional<int> WorkOrder::technicianId() const { return m_technicianId; }
void WorkOrder::setTechnicianId(std::optional<int> technicianId) { m_technicianId = technicianId; }

QDateTime WorkOrder::scheduledStart() const { return m_scheduledStart; }
void WorkOrder::setScheduledStart(const QDateTime &scheduledStart) { m_scheduledStart = scheduledStart; }

QString WorkOrder::serviceAddress() const { return m_serviceAddress; }
void WorkOrder::setServiceAddress(const QString &serviceAddress) { m_serviceAddress = serviceAddress; }

QString WorkOrder::serviceDescription() const { return m_serviceDescription; }
void WorkOrder::setServiceDescription(const QString &serviceDescription) { m_serviceDescription = serviceDescription; }

QString WorkOrder::notes() const { return m_notes; }
void WorkOrder::setNotes(const QString &notes) { m_notes = notes; }

WorkOrder::Status WorkOrder::status() const { return m_status; }
void WorkOrder::setStatus(Status status) { m_status = status; }

QString WorkOrder::statusToString(Status status)
{
    switch (status) {
    case Status::Unscheduled: return "Unscheduled";
    case Status::Scheduled: return "Scheduled";
    case Status::InProgress: return "In Progress";
    case Status::Completed: return "Completed";
    case Status::Cancelled: return "Cancelled";
    }
    return "Unscheduled";
}

WorkOrder::Status WorkOrder::statusFromString(const QString &status)
{
    if (status == "Scheduled") return Status::Scheduled;
    if (status == "In Progress") return Status::InProgress;
    if (status == "Completed") return Status::Completed;
    if (status == "Cancelled") return Status::Cancelled;
    return Status::Unscheduled;
}
