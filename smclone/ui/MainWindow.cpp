#include "MainWindow.h"

#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "database/DatabaseManager.h"
#include "ui/CreateAccountScreen.h"
#include "ui/CreateWorkOrderScreen.h"
#include "ui/ScheduleScreen.h"
#include "ui/ViewAccountScreen.h"
#include "ui/ViewWorkOrderScreen.h"

MainWindow::MainWindow(DatabaseManager *database, QWidget *parent) : QMainWindow(parent), m_database(database){
	setWindowTitle("Alpenmonster")
	auto *central = new QWidget;
	auto *rootLayout = new QHBoxLayout(central);
	rootLayout->setContentsMargins(0, 0, 0, 0);
	rootLayout->setSpacing(0);

	auto *sidebar = new QFrame;
	sidebar->setMinimumWidth(230);
	sidebar->setMaximumWidth(260);
	sidebar->setFrameShape(QFrame::StyledPanel);

	auto *sidebarLayout = new QVBoxLayout(sidebar);
	auto *menuTitle = new QLabel("Main Menu");
	QFont menuFont = menuTitle->font();
	menuFont.setPointSize(18);
	menuFont.setBold(true);
	menuTitle->setFont(menuFont);
	menuTitle->setAlignment(Qt::AlignCenter);
	sidebarLayout->addWidget(menuTitle);

	auto addSection = [sidebarLayout](const QString &title, const QList<QPushButton *> &buttons){
		auto *label = new QLabel(title);
		QFont font = label->font();
		font.setBold(true);
		font.setPointSize(12);
		label->setFont(font);
		sidebarLayout->addSpacing(10);
		sidebarLayout->addWidget(label);
		for(QPushButton *button : buttons){
			sidebarLayout->addWidget(button);
		}
	};

	auto *newAccountButton = new QPushButton("Create New Account");
    auto *lookupAccountButton = new QPushButton("Lookup Account");
    auto *newWorkOrderButton = new QPushButton("Create New Work Order");
    auto *lookupWorkOrderButton = new QPushButton("Lookup Work Order");
    auto *todayButton = new QPushButton("Go to Today");
    auto *specificDateButton = new QPushButton("Go to Specific Date");

    addSection("Accounts", {newAccountButton, lookupAccountButton});
    addSection("Work Orders", {newWorkOrderButton, lookupWorkOrderButton});
    addSection("Schedule", {todayButton, specificDateButton});
    sidebarLayout->addStretch();

    auto *dbLabel = new QLabel("SQLite database:\n" + m_database->databasePath());
    dbLabel->setWordWrap(true);
    dbLabel->setStyleSheet("color: gray; font-size: 9px;");
    sidebarLayout->addWidget(dbLabel);

    m_stack = new QStackedWidget;
    m_createAccountScreen = new CreateAccountScreen(m_database);
    m_viewAccountScreen = new ViewAccountScreen(m_database);
    m_createWorkOrderScreen = new CreateWorkOrderScreen(m_database);
    m_viewWorkOrderScreen = new ViewWorkOrderScreen(m_database);
    m_scheduleScreen = new ScheduleScreen(m_database);

    m_stack->addWidget(m_createAccountScreen);
    m_stack->addWidget(m_viewAccountScreen);
    m_stack->addWidget(m_createWorkOrderScreen);
    m_stack->addWidget(m_viewWorkOrderScreen);
    m_stack->addWidget(m_scheduleScreen);

    rootLayout->addWidget(sidebar);
    rootLayout->addWidget(m_stack, 1);
    setCentralWidget(central);

    connect(newAccountButton, &QPushButton::clicked, this, &MainWindow::showCreateAccount);
    connect(lookupAccountButton, &QPushButton::clicked, this, &MainWindow::showAccounts);
    connect(newWorkOrderButton, &QPushButton::clicked, this, &MainWindow::showCreateWorkOrder);
    connect(lookupWorkOrderButton, &QPushButton::clicked, this, &MainWindow::showWorkOrders);
    connect(todayButton, &QPushButton::clicked, this, &MainWindow::showScheduleToday);
    connect(specificDateButton, &QPushButton::clicked, this, &MainWindow::chooseScheduleDate);

    connect(m_viewAccountScreen, &ViewAccountScreen::editCustomerRequested, this, &MainWindow::editCustomer);
    connect(m_viewWorkOrderScreen, &ViewWorkOrderScreen::editWorkOrderRequested, this, &MainWindow::editWorkOrder);
    connect(m_scheduleScreen, &ScheduleScreen::editWorkOrderRequested, this, &MainWindow::editWorkOrder);
    connect(m_createAccountScreen, &CreateAccountScreen::customerSaved, this, &MainWindow::handleCustomerSaved);
    connect(m_createWorkOrderScreen, &CreateWorkOrderScreen::workOrderSaved, this, &MainWindow::handleWorkOrderSaved);

    showScheduleToday();
    statusBar()->showMesage("Ready");
}

void MainWindow::showCreateAccount(){
	m_createAccountScreen->beginNewCustomer();
	m_stack->setCurrentWidget(m_createAccountScreen);
}

void MainWindow::showAccounts(){
	m_viewAccountScreen->refresh();
	m_stack->setCurrentWidget(m_viewAccountScreen);
}


void MainWindow::showCreateWorkOrder(){
    m_createWorkOrderScreen->beginNewWorkOrder();
    m_stack->setCurrentWidget(m_createWorkOrderScreen);
}

void MainWindow::showWorkOrders(){
    m_viewWorkOrderScreen->refresh();
    m_stack->setCurrentWidget(m_viewWorkOrderScreen);
}

void MainWindow::showScheduleToday(){
    m_scheduleScreen->goToToday();
    m_stack->setCurrentWidget(m_scheduleScreen);
}

void MainWindow::chooseScheduleDate(){
    QDialog dialog(this);
    dialog.setWindowTitle("Go to Specific Date");

    auto *layout = new QVBoxLayout(&dialog);
    auto *dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("MMMM d, yyyy");
    layout->addWidget(new QLabel("Select a date:"));
    layout->addWidget(dateEdit);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        m_scheduleScreen->setDate(dateEdit->date());
        m_stack->setCurrentWidget(m_scheduleScreen);
    }
}

void MainWindow::editCustomer(int customerId) {
    m_createAccountScreen->editCustomer(customerId);
    m_stack->setCurrentWidget(m_createAccountScreen);
}

void MainWindow::editWorkOrder(int workOrderId){
    m_createWorkOrderScreen->editWorkOrder(workOrderId);
    m_stack->setCurrentWidget(m_createWorkOrderScreen);
}

void MainWindow::handleCustomerSaved(){
    m_viewAccountScreen->refresh();
    m_createWorkOrderScreen->refreshCustomers();
    statusBar()->showMessage("Customer saved", 3000);
}

void MainWindow::handleWorkOrderSaved(){
    m_viewWorkOrderScreen->refresh();
    m_scheduleScreen->refresh();
    statusBar()->showMessage("Work order saved", 3000);
}
