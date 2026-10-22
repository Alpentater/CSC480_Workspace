#pragma once

#include <QDateTime>
#include <QString>
#include <optional>

class WorkOrder
{
public:
    enum class Status {
        Unscheduled,
        Scheduled,
        InProgress,
        Completed,
        Cancelled
    };

    WorkOrder() = default;

    int id() const;
    void setId(int id);

    int customerId() const;
    void setCustomerId(int customerId);

    std::optional<int> technicianId() const;
    void setTechnicianId(std::optional<int> technicianId);

    QDateTime scheduledStart() const;
    void setScheduledStart(const QDateTime &scheduledStart);

    QString serviceAddress() const;
    void setServiceAddress(const QString &serviceAddress);

    QString serviceDescription() const;
    void setServiceDescription(const QString &serviceDescription);

    QString notes() const;
    void setNotes(const QString &notes);

    Status status() const;
    void setStatus(Status status);

    static QString statusToString(Status status);
    static Status statusFromString(const QString &status);

private:
    int m_id = 0;
    int m_customerId = 0;
    std::optional<int> m_technicianId;
    QDateTime m_scheduledStart;
    QString m_serviceAddress;
    QString m_serviceDescription;
    QString m_notes;
    Status m_status = Status::Unscheduled;
};
