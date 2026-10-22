#include "ScheduleScreen.h"

#include <QDateEdit>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include "database/DatabaseManager.h"
#include "ui/DragDropWidgets.h"

namespace {
	constexpr int StartHour = 8;
	constexpr int EndHour = 18;
	constexpr int MinutesPerSlot = 30;
}

ScheduleScreen::ScheduleScreen(DatabaseManager *database, QWidget *parent) : QWidget(parent), m_database(database){
	auto *previousButton = new QPushButton("< Previous");
	auto *todayButton = new QPushButton("Today");
	auto *nextButton = new QPushButton("Next >");

	m_dateEdit = new QDateEdit(QDate::currentDate());
	m_dateEdit->setCalenderPopup(true);
	m_dateEdit->setDisplayFormat("MMMM d, yyyy");

	m_dateTitle = new QLabel;
	QFont titleFont = m_dateTitle->font();
	titleFont.setPointSize(18);
	titleFont.setBold(true);
	m_dateTitle->setFont(titleFont);

	auto *topBar = new QHBoxLayout;
	topBar->addWidget(previousButton);
	topBar->addWidget(todayButton);
	topBar->addWidget(nextButton);
	topBar->addSpacing(20);
	topBar->addWidget(new QLabel("Date:"));
	topBar->addWidget(m_dateEdit);
	topBar->addStretch();
	topBar->addWidget(m_dateTitle);

	m_scheduleTable = new ScheduleTableWidget;
	m_scheduleTable->verticalHeader()->setVisible(true);
	m_scheduleTable->horizontalHeader()->SetSectionResizeMode(QHeaderView::Stretch);
	m_scheduleTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

	const int rows = ((EndHour - StartHour)* 60)/MinutesPerSlot + 1;
	m_scheduleTable->SetRowCount(rows);
	for(int row = 0; row < rows; ++row){
		const QTime time = timeForRow(row);
		m_scheduleTable->setVerticalHeaderItem(row, new QTableWidgetItem(time.toString("h:mm AP")));
		m_scheduleTable->setRowHeight(row, 42);
	}

	auto *deckTitle = new QLabel("Deck\nUnscheduled Work Orders");
	QFont deckFont = deckTitle->font();
	deckFont.setBold(true);
	deckTitle->setFont(deckFont);
	deckTitle->setAlignment(Qt::AlignCenter);

	m_deck = new DeckListWidget;
	m_deck->setMinimumWidth(270);

	auto *deckLayout = new QVBoxLayout;
	deckLayout->addWidget(deckTitle);
	deckLayout->addWidget(m_deck);

	auto *contentLayout = new QHBoxLayout;
	contentLayout->addWidget(m_scheduleTable, 1);
	contentLayout->addLayout(deckLayout);

	auto *layout = new QVBoxLayout(this);
	layout->addLayout(topBar);
	layout->addLayout(contentLayout, 1);

	connect(previousButton, &QPushButton::clicked, this, &ScheduleScreen::previousDay);
    connect(todayButton, &QPushButton::clicked, this, &ScheduleScreen::goToToday);
    connect(nextButton, &QPushButton::clicked, this, &ScheduleScreen::nextDay);
    connect(m_dateEdit, &QDateEdit::dateChanged, this, &ScheduleScreen::selectedDateChanged);
    connect(m_scheduleTable, &ScheduleTableWidget::workOrderDropped, this, &ScheduleScreen::scheduleWorkOrder);
    connect(m_deck, &DeckListWidget::workOrderReturnedToDeck, this, &ScheduleScreen::returnWorkOrderToDeck);
    connect(m_scheduleTable, &QTableWidget::cellDoubleClicked, this, &ScheduleScreen::openSelectedWorkOrder);
    connect(m_deck, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item, int) { openDeckWorkOrder(item); });

    setDate(QDate::currentDate());
}

void ScheduleScreen::setDate(const QDate &date){
	if(!date.isValid()){
		return;
	}
	m_schedule.setCurrentDate(date);
	m_dateEdit->blockSignals(true);
	m_dateEdit->setDate(date);
	m_dateEdit->blockSignals(false);
	refresh();
}

void ScheduleScreen::goToToday(){ setDate(QDate::currentDate()); }

void ScheduleScreen::previousDay(){ setDate(m_schedule.currentDate().addDays(-1)); }

void ScheduleScreen::nextDay(){ setDate(m_schedule.currentDate().addDays(1)); }

void ScheduleScreen::selectedDateChanged(const QDate &date){ setDate(date); }

void ScheduleScreen::refresh(){
	m_dateTitle->setText(m_schedule.currentDate().toString("dddd, MMMM d, yyyy"));

	m_techicians = m_database->techicians();
	m_scheduleTable->clearContents();
	m_scheduleTable->setColumnCount(static_cast<int>(m_techicians.size()));

	QStringList headers;
	for(const Technician &tech : m_techicians){ headers << tech.name(); }
	m_scheduleTable->setHorizontalHeaderLabels(headers);

	const auto orders = m_database->workOrdersForDate(m_schedule.currentDate());
	m_schedule.setWorkOrders(orders);

	for(const WorkOrder &order : orders){
		if(!order.technicianId() || !order.scheduledStart().isValid()) continue;

		const int column = columnForTechnician(*order.technicianId());
		const int row = rowForTime(order.scheduledStart().time());
		if(column < 0 || row < 0 || row >= m_scheduleTable->rowCount()) continue;

		const QString text = QString("#%1\n%2\n%3").arg(order.id()).arg(customerName(order.customerId())).arg(order.serviceAddress());

		auto *item = new QTableWidgetItem(text);
		item->setData(Qt::UserRole, order.id());
		item->setToolTip(order.serviceDescription() + "\n" + WorkOrder::statusToString(order.status()));
		m_scheduleTable->setItem(row, column, item);
	}

	m_deck->clear();
	for(const WorkOrder &order : m_database->unscheduledWorkOrders()){
		const QString text QString("#%1 | %2\n%3\n%4").arg(order.id()).arg(customerName(order.customerId())).arg(order.serviceAddress()).arg(order.serviceDescription());

		auto *item = new QListWidgetItem(text);
		item->setData(Qt::UserRole, order.id());
        item->setToolTip("Drag this work order onto a technician and time slot.");
        m_deck->addItem(item);
	}
}

void ScheduleScreen::scheduleWorkOrder(int workOrderId, int row, int column) {
    if (column < 0 || column >= static_cast<int>(m_technicians.size())) return;

    if (targetCellOccupied(row, column, workOrderId)) {
        QMessageBox::warning(this, "Schedule Conflict","That technician already has a work order in this time slot.");
        refresh();
        return;
    }

    const Technician &technician = m_technicians.at(column);
    const QDateTime start(m_schedule.currentDate(), timeForRow(row));

    if (!m_database->scheduleWorkOrder(workOrderId, technician.id(), start)) {
        QMessageBox::critical(this,"Scheduling Error","The work order could not be scheduled. No changes were saved.\n\n" +m_database->lastError());
    }
    refresh();
}

void ScheduleScreen::returnWorkOrderToDeck(int workOrderId){
    if (!m_database->unscheduleWorkOrder(workOrderId)) {
        QMessageBox::critical(this, "Scheduling Error", "The work order could not be returned to the Deck.\n\n" + m_database->lastError());
    }
    refresh();
}

void ScheduleScreen::openSelectedWorkOrder(int row, int column){
    QTableWidgetItem *item = m_scheduleTable->item(row, column);
    if (!item) {return;}
    const int id = item->data(Qt::UserRole).toInt();
    if (id > 0) emit editWorkOrderRequested(id);
}

void ScheduleScreen::openDeckWorkOrder(QListWidgetItem *item){
    if (!item) { return; }
    const int id = item->data(Qt::UserRole).toInt();
    if (id > 0) {emit editWorkOrderRequested(id); }
}

QTime ScheduleScreen::timeForRow(int row) const { return QTime(StartHour, 0).addSecs(row * MinutesPerSlot * 60); }

int ScheduleScreen::rowForTime(const QTime &time) const {
    const int minutes = QTime(StartHour, 0).secsTo(time) / 60;
    if (minutes < 0) {return -1;}
    return minutes / MinutesPerSlot;
}

int ScheduleScreen::columnForTechnician(int technicianId) const {
    for (int i = 0; i < static_cast<int>(m_technicians.size()); ++i) {
        if (m_technicians.at(i).id() == technicianId) return i;
    }
    return -1;
}

QString ScheduleScreen::customerName(int customerId) const {
    Customer customer;
    if (m_database->customerById(customerId, customer)) return customer.name();
    return QString("Customer #%1").arg(customerId);
}

bool ScheduleScreen::targetCellOccupied(int row, int column, int movingWorkOrderId) const {
    QTableWidgetItem *item = m_scheduleTable->item(row, column);
    if (!item) return false;
    return item->data(Qt::UserRole).toInt() != movingWorkOrderId;
}
