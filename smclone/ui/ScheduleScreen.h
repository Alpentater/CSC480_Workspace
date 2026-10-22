#pragma once

#include <QDate>
#include <QTime>
#include <QWidget>
#include <vector>

#include "models/Schedule.h"
#include "models/Technician.h"

class DatabaseManager;
class QDateEdit;
class QLabel;
class QListWidgetItem;
class DeckListWidget;
class ScheduleTableWidget;

class ScheduleScreen : public QWidget {
	Q_OBJECT

public:
	explicit ScheduleScreen(DatabaseManager *database, QWidget *parent = nullptr);

	void setDate(const QDate &date);
	void goToToday();
	void refresh();

signals:
	void editWorkOrderRequested(int workOrderId);

private slots:
    void previousDay();
    void nextDay();
    void selectedDateChanged(const QDate &date);
    void scheduleWorkOrder(int workOrderId, int row, int column);
    void returnWorkOrderToDeck(int workOrderId);
    void openSelectedWorkOrder(int row, int column);
    void openDeckWorkOrder(QListWidgetItem *item);

private:
    QTime timeForRow(int row) const;
    int rowForTime(const QTime &time) const;
    int columnForTechnician(int technicianId) const;
    QString customerName(int customerId) const;
    bool targetCellOccupied(int row, int column, int movingWorkOrderId) const;

    DatabaseManager *m_database;
    Schedule m_schedule;
    std::vector<Technician> m_technicians;

    QDateEdit *m_dateEdit;
    QLabel *m_dateTitle;
    ScheduleTableWidget *m_scheduleTable;
    DeckListWidget *m_deck;
};