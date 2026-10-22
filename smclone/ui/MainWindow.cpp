//Header file for the application's main window.
#include "MainWindow.h"

//Qt classes used to build the main application interface, sidebar navigation, date-selection dialog, and status bar.
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
#include <QDate>

//Provides access to the application's SQLite database.
#include "database/DatabaseManager.h"

//Headers for each of the primary application screens.
#include "ui/CreateAccountScreen.h"
#include "ui/CreateWorkOrderScreen.h"
#include "ui/ScheduleScreen.h"
#include "ui/ViewAccountScreen.h"
#include "ui/ViewWorkOrderScreen.h"

//Constructor for the MainWindow.
//The MainWindow acts as the main container for the program. 
//It contains the persistent sidebar navigation and a QStackedWidget that switches between the different application screens.
MainWindow::MainWindow(DatabaseManager *database, QWidget *parent) : QMainWindow(parent), m_database(database){
    
    //Set the title displayed at the top of the application window.
	setWindowTitle("Alpenmonster");

    //Create the central widget that will hold the entire interface.
	auto *central = new QWidget;

    //Create a horizontal layout.
    //The sidebar will appear on the left and the currently selected application screen will appear on the right.
	auto *rootLayout = new QHBoxLayout(central);
	rootLayout->setContentsMargins(0, 0, 0, 0); //Remove unnecessary space around the outside of the window.
	rootLayout->setSpacing(0); //Remove spacing between the sidebar and the main screen.

    //reate the persistent navigation sidebar.
	auto *sidebar = new QFrame;
    //Limit the sidebar width so it stays a consistent size and give it a visible frame.
	sidebar->setMinimumWidth(230);
	sidebar->setMaximumWidth(260);
	sidebar->setFrameShape(QFrame::StyledPanel);

    //Create the vertical layout used inside the sidebar.
	auto *sidebarLayout = new QVBoxLayout(sidebar);

    //Create the "Main Menu" heading... making the font size and making it bold.
	auto *menuTitle = new QLabel("Main Menu");
	QFont menuFont = menuTitle->font();
	menuFont.setPointSize(18);
	menuFont.setBold(true);
	menuTitle->setFont(menuFont);

    //Center the heading inside the sidebar and add the heading to the sidebar.
	menuTitle->setAlignment(Qt::AlignCenter);
	sidebarLayout->addWidget(menuTitle);

    //Below is a helper function used to create sections in the sidebar.
    //Each section receives a title such as "Accounts" and  a list of buttons belonging to that section
    //Using this lambda avoids repeating the same layout code for every section of the menu.
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

    //Navigation buttons.
	auto *newAccountButton = new QPushButton("Create New Account");
    auto *lookupAccountButton = new QPushButton("Lookup Account");
    auto *newWorkOrderButton = new QPushButton("Create New Work Order");
    auto *lookupWorkOrderButton = new QPushButton("Lookup Work Order");
    auto *todayButton = new QPushButton("Go to Today");
    auto *specificDateButton = new QPushButton("Go to Specific Date");

    //Add the buttons to their corresponding sidebar sections.
    addSection("Accounts", {newAccountButton, lookupAccountButton});
    addSection("Work Orders", {newWorkOrderButton, lookupWorkOrderButton});
    addSection("Schedule", {todayButton, specificDateButton});

    //Push the database information toward the bottom of the sidebar.
    sidebarLayout->addStretch();

    //Display the current SQLite database file location. 
    //This is mainly useful during development and debugging so the database file can easily be located.
    auto *dbLabel = new QLabel("SQLite database:\n" + m_database->databasePath());
    dbLabel->setWordWrap(true);
    dbLabel->setStyleSheet("color: gray; font-size: 9px;");
    sidebarLayout->addWidget(dbLabel);

    //QStackedWidget holds all of the application's major screens. Only one screen is visible at a time.
    m_stack = new QStackedWidget;

    //Create each major application screen and pass the shared DatabaseManager to screens that need database access.
    m_createAccountScreen = new CreateAccountScreen(m_database);
    m_viewAccountScreen = new ViewAccountScreen(m_database);
    m_createWorkOrderScreen = new CreateWorkOrderScreen(m_database);
    m_viewWorkOrderScreen = new ViewWorkOrderScreen(m_database);
    m_scheduleScreen = new ScheduleScreen(m_database);

    //Add each screen to the stacked widget.
    m_stack->addWidget(m_createAccountScreen);
    m_stack->addWidget(m_viewAccountScreen);
    m_stack->addWidget(m_createWorkOrderScreen);
    m_stack->addWidget(m_viewWorkOrderScreen);
    m_stack->addWidget(m_scheduleScreen);

    //Add the sidebar and screen container to the main layout.The stretch value of 1 allows the main screen to use the remaining available window space.
    rootLayout->addWidget(sidebar);
    rootLayout->addWidget(m_stack, 1);
    setCentralWidget(central);

    //Sidebar widget signals for navigating. for example, the first line here in this block opens the 'Create new account' screen.
    connect(newAccountButton, &QPushButton::clicked, this, &MainWindow::showCreateAccount);
    connect(lookupAccountButton, &QPushButton::clicked, this, &MainWindow::showAccounts);
    connect(newWorkOrderButton, &QPushButton::clicked, this, &MainWindow::showCreateWorkOrder);
    connect(lookupWorkOrderButton, &QPushButton::clicked, this, &MainWindow::showWorkOrders);
    connect(todayButton, &QPushButton::clicked, this, &MainWindow::showScheduleToday);
    connect(specificDateButton, &QPushButton::clicked, this, &MainWindow::chooseScheduleDate);

    //Screen-to-Screen Signals
    connect(m_viewAccountScreen, &ViewAccountScreen::editCustomerRequested, this, &MainWindow::editCustomer);
    connect(m_viewWorkOrderScreen, &ViewWorkOrderScreen::editWorkOrderRequested, this, &MainWindow::editWorkOrder);
    connect(m_scheduleScreen, &ScheduleScreen::editWorkOrderRequested, this, &MainWindow::editWorkOrder);
    connect(m_createAccountScreen, &CreateAccountScreen::customerSaved, this, &MainWindow::handleCustomerSaved);
    connect(m_createWorkOrderScreen, &CreateWorkOrderScreen::workOrderSaved, this, &MainWindow::handleWorkOrderSaved);

    //The application should always start on today's schedule.
    showScheduleToday();
    statusBar()->showMessage("Ready");
}

//Opens the Create Account screen and prepares it for a brand-new customer.
void MainWindow::showCreateAccount(){
	m_createAccountScreen->beginNewCustomer();
	m_stack->setCurrentWidget(m_createAccountScreen);
}

//Opens the customer lookup/view screen.
void MainWindow::showAccounts(){
	m_viewAccountScreen->refresh();
	m_stack->setCurrentWidget(m_viewAccountScreen);
}

//Opens the Create Work Order screen.
void MainWindow::showCreateWorkOrder(){
    m_createWorkOrderScreen->beginNewWorkOrder();
    m_stack->setCurrentWidget(m_createWorkOrderScreen);
}

//Opens the Work Order lookup/view screen.
void MainWindow::showWorkOrders(){
    m_viewWorkOrderScreen->refresh();
    m_stack->setCurrentWidget(m_viewWorkOrderScreen);
}

//Opens the Schedule screen and returns it to today's date.
void MainWindow::showScheduleToday(){
    m_scheduleScreen->goToToday();
    m_stack->setCurrentWidget(m_scheduleScreen);
}

//Displays a dialog that allows the user to navigate directly to a specific schedule date.
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

//Opens an existing customer in the account-editing screen.
void MainWindow::editCustomer(int customerId) {
    m_createAccountScreen->editCustomer(customerId);
    m_stack->setCurrentWidget(m_createAccountScreen);
}

//Opens an existing work order in the work-order editing screen.
void MainWindow::editWorkOrder(int workOrderId){
    m_createWorkOrderScreen->editWorkOrder(workOrderId);
    m_stack->setCurrentWidget(m_createWorkOrderScreen);
}

//Called after a customer has successfully been saved.
void MainWindow::handleCustomerSaved(){
    m_viewAccountScreen->refresh();
    m_createWorkOrderScreen->refreshCustomers();
    statusBar()->showMessage("Customer saved", 3000);
}

//Called after a work order has successfully been saved.
void MainWindow::handleWorkOrderSaved(){
    m_viewWorkOrderScreen->refresh();
    m_scheduleScreen->refresh();
    statusBar()->showMessage("Work order saved", 3000);
}
