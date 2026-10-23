//Header file for the ScheduleScreen class.
#include "ScheduleScreen.h"

//Qt classes used to build and manage the schedule interface.
#include <QDateEdit>
#include <QDateTime>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QStringList>
#include <QTableWidgetItem>
#include <QVBoxLayout>

//Provides access to database operations.
#include "database/DatabaseManager.h"

//Custom widgets that provide drag-and-drop functionality for the schedule table and unscheduled Deck.
#include "ui/DragDropWidgets.h"

//Values inside this anonymous namespace are only available within this source file.
namespace {
	constexpr int StartHour = 8; //The schedule begins at 8am...
	constexpr int EndHour = 18;  //...and ends at 6pm
	constexpr int MinutesPerSlot = 30; //Each row of the schedule represents 30 minutes.
}

//Constructor for the ScheduleScreen. This screen displays: date navigation controls,technician columns, ...
//...time slots, scheduled work orders, and the Deck of unscheduled work orders.
ScheduleScreen::ScheduleScreen(DatabaseManager *database, QWidget *parent) : QWidget(parent), m_database(database){

	//Buttons used to move between schedule dates.
	auto *previousButton = new QPushButton("< Previous");
	auto *todayButton = new QPushButton("Today");
	auto *nextButton = new QPushButton("Next >");

	//Create a date picker starting on today's date.
	m_dateEdit = new QDateEdit(QDate::currentDate());

	//Allow the user to select a date from a popup calendar.
	m_dateEdit->setCalendarPopup(true);

	//Formatting the date number value to a readable format.
	m_dateEdit->setDisplayFormat("MMMM d, yyyy");

	//Create the large date title shown above the schedule.
	m_dateTitle = new QLabel;

	//Increase the date title font size and make it bold.
	QFont titleFont = m_dateTitle->font();
	titleFont.setPointSize(18);
	titleFont.setBold(true);
	m_dateTitle->setFont(titleFont);

	//Create the horizontal layout at the top of the screen.
	auto *topBar = new QHBoxLayout;

	//Add the date-navigation buttons.
	topBar->addWidget(previousButton);
	topBar->addWidget(todayButton);
	topBar->addWidget(nextButton);

	//Add some visual separation before the date picker.
	topBar->addSpacing(20);
	
	//Add the date picker controls.
	topBar->addWidget(new QLabel("Date:"));
	topBar->addWidget(m_dateEdit);

	//Push the large date title toward the right side.
	topBar->addStretch();
	topBar->addWidget(m_dateTitle);

	//Create the custom table used to display scheduled work orders.
	m_scheduleTable = new ScheduleTableWidget;

	//Show the vertical time labels.
	m_scheduleTable->verticalHeader()->setVisible(true);

	//Stretch each technician column evenly across the table.
	m_scheduleTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	//You should be able to directly edit the text inside schedule cells.
	m_scheduleTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

	// Calculate the number of 30-minute rows required between the start and end of the work day.
	const int rows = ((EndHour - StartHour)* 60)/MinutesPerSlot + 1;

	//Create the required number of schedule rows.
	m_scheduleTable->setRowCount(rows);

	//Label each row with its corresponding appointment time.
	for(int row = 0; row < rows; ++row){
		//Convert the row number into a QTime and add the time label to the left side of the schedule.
		const QTime time = timeForRow(row);
		m_scheduleTable->setVerticalHeaderItem(row, new QTableWidgetItem(time.toString("h:mm AP")));

		// Make each time slot tall enough to clearly display basic work-order information.
		m_scheduleTable->setRowHeight(row, 42);
	}

 	//Create the title displayed above the Deck.
	auto *deckTitle = new QLabel("Deck\nUnscheduled Work Orders");

	//Make the Deck title bold.
	QFont deckFont = deckTitle->font();
	deckFont.setBold(true);
	deckTitle->setFont(deckFont);

	//Center the Deck title.
	deckTitle->setAlignment(Qt::AlignCenter);

	//Create the custom Deck list widget. Give the Deck enough width to display work-order information clearly.
	m_deck = new DeckListWidget;
	m_deck->setMinimumWidth(270);

	//Create the vertical layout for the Deck.
	auto *deckLayout = new QVBoxLayout;
	deckLayout->addWidget(deckTitle);
	deckLayout->addWidget(m_deck);
	
	//Create the vertical layout for the Deck.
	auto *contentLayout = new QHBoxLayout;
	contentLayout->addWidget(m_scheduleTable, 1);
	contentLayout->addLayout(deckLayout);

	//Create the main vertical layout for this screen.
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
    connect(m_deck, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) { openDeckWorkOrder(item); });

    setDate(QDate::currentDate());
}

//Changes the schedule to the specified date.
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

//Return the schedule to today's date.
void ScheduleScreen::goToToday(){ setDate(QDate::currentDate()); }

//Move the schedule backward by one day.
void ScheduleScreen::previousDay(){ setDate(m_schedule.currentDate().addDays(-1)); }

//Move the schedule ahead one day.
void ScheduleScreen::nextDay(){ setDate(m_schedule.currentDate().addDays(1)); }

//Called when the user manually selects another date.
void ScheduleScreen::selectedDateChanged(const QDate &date){ setDate(date); }

//Reloads all information displayed on the Schedule screen.
void ScheduleScreen::refresh(){
	m_dateTitle->setText(m_schedule.currentDate().toString("dddd, MMMM d, yyyy"));

	m_technicians = m_database->technicians();
	m_scheduleTable->clearContents();
	m_scheduleTable->setColumnCount(static_cast<int>(m_technicians.size()));

	QStringList headers;
	for(const Technician &tech : m_technicians){ headers << tech.name(); }
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
		const QString text = QString("#%1 | %2\n%3\n%4").arg(order.id()).arg(customerName(order.customerId())).arg(order.serviceAddress()).arg(order.serviceDescription());

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

//Opens a work order when the user double-clicks an item on the schedule.
void ScheduleScreen::returnWorkOrderToDeck(int workOrderId){
    if (!m_database->unscheduleWorkOrder(workOrderId)) {
        QMessageBox::critical(this, "Scheduling Error", "The work order could not be returned to the Deck.\n\n" + m_database->lastError());
    }
    refresh();
}

//Opens a work order when the user double-clicks an item inside the Deck.
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

// Converts a schedule row into an appointment time. For example: Row 0 = 8:00 AM or Row 1 = 8:30 AM
QTime ScheduleScreen::timeForRow(int row) const { return QTime(StartHour, 0).addSecs(row * MinutesPerSlot * 60); }

//Converts an appointment time into a schedule row number.
int ScheduleScreen::rowForTime(const QTime &time) const {
    const int minutes = QTime(StartHour, 0).secsTo(time) / 60;
    if (minutes < 0) {return -1;}
    return minutes / MinutesPerSlot;
}

//Finds which schedule column belongs to a technician.
int ScheduleScreen::columnForTechnician(int technicianId) const {
    for (int i = 0; i < static_cast<int>(m_technicians.size()); ++i) {
        if (m_technicians.at(i).id() == technicianId) return i;
    }
    return -1;
}

//Retrieves the customer's name for display on a work order.
QString ScheduleScreen::customerName(int customerId) const {
    Customer customer;
    if (m_database->customerById(customerId, customer)) return customer.name();
    return QString("Customer #%1").arg(customerId);
}

//Determines whether another work order already occupied the requested technician/time cell.
bool ScheduleScreen::targetCellOccupied(int row, int column, int movingWorkOrderId) const {
    QTableWidgetItem *item = m_scheduleTable->item(row, column);
    if (!item) return false;
    return item->data(Qt::UserRole).toInt() != movingWorkOrderId;
}
