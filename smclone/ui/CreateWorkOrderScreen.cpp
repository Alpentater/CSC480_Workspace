//Header file for the CreateWorkOrderScreen class.
#include "CreateWorkOrderScreen.h"

//Qt layout and widget classes used to build the form
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

//Header that gives the ability to use database operations.
#include "database/DatabaseManager.h"

CreateWorkOrderScreen::CreateWorkOrderScreen(DatabaseManager *database, QWidget *parent) : QWidget(parent), m_database(database){
	//Creating the title label that is at the top of the screen.
	m_titleLabel = new QLabel("Create New Work Order");

	//Increase the title font size and make it bold.
	QFont titleFont = m_titleLabel->font();
	titleFont.setPointSize(18);
	titleFont.setBold(true);
	m_titleLabel->setFont(titleFont);

	//Creating the input controls used for work order information.
	m_customerCombo = new QComboBox;
	m_addressEdit = new QLineEdit;
	m_descriptionEdit = new QLineEdit;
	m_noteEdit = new QTextEdit;

	//Limiting the heighn of the notes box so it dows not take up an excessive amount of space on the screen.
	m_noteEdit->setMaximumHeight(130);

	//Create the status dropdown and add each possible WorkOrder state.
	m_statusCombo = new QComboBox;
	m_statusCombo->addItems({"Unscheduled", "Scheduled", "In Progress", "Completed", "Cancelled"});
	
	//Creating the save button.
	m_saveButton = new QPushButton("Save Work Order");

	//Create the form layout and add each input field.
	auto *form = new QFormLayout;
	form->addRow("Customer*", m_customerCombo);
	form->addRow("Service Address*", m_addressEdit);
	form->addRow("Service Description", m_descriptionEdit);
	form->addRow("Notes", m_noteEdit);
	form->addRow("Status", m_statusCombo);

	//Create the main vertical layout for this screen.
	auto *layout = new QVBoxLayout(this);

	//Add the title, form, and save button to the screen.
	layout->addWidget(m_titleLabel);
	layout->addLayout(form);
	layout->addWidget(m_saveButton, 0, Qt::AlignLeft);
	layout->addStretch(); //pushing the form to the top of window.

	// When the Save Work Order button is clicked, call the saveWorkOrder function.
	connect(m_saveButton, &QPushButton::clicked, this, &CreateWorkOrderScreen::saveWorkOrder);
	//When the selected customer changes, call customerChanged().
    //This allows the customer's service address to automatically populate the work-order address field.
	connect(m_customerCombo, &QComboBox::currentIndexChanged, this, &CreateWorkOrderScreen::customerChanged);

	//Loading the available customers form the database and prepare the form to create a new work order.
	refreshCustomers();
	beginNewWorkOrder();
}

//Refreshes the customer dropdown using the customer stored in the database.
void CreateWorkOrderScreen::refreshCustomers(){

	//Save the currently selected customer ID so the same customer can be selected again afer the list refreshes.
	const int previousId = m_customerCombo->currentData().toInt();
	//temporarily stop signals while modifying the dropbown.
	m_customerCombo->blockSignals(true);
	//Remove all existing entries from the dropdown.
	m_customerCombo->clear();
	//Add a default option representing no selected customer. The value 0 indicates that no valid customer has been chosen.
	m_customerCombo->addItem("Select a customer...", 0);

	//Retrieve all customers from the database and add them to the dropdown using their name and Customer ID.
	for(const Customer &customer : m_database->customers()){
		m_customerCombo->addItem(customer.name(), customer.id());
	}

	//Search for the previosuly selected customer.
	const int restored = m_customerCombo->findData(previousId);
	//Restore the previous selection if that customer still exists.
	if(restored >= 0) { m_customerCombo->setCurrentIndex(restored); }
	//Re-enable signals after the dropdown has been refreshed.
	m_customerCombo->blockSignals(false);
}

//Prepares the screeen for createion of a brand-new work order.
void CreateWorkOrderScreen::beginNewWorkOrder(){
	//An ID of 0 means that a new work order is being created instead of editing an existing one.
	m_editingWorkOrderId = 0;

	//Reset the title and save button text.
	m_titleLabel->setText("Create New Work Order");
	m_saveButton->setText("Save Work Order");

	//Refresh the available customer list.
	refreshCustomers();

	//Reset the customer dropdown to its default option.
	m_customerCombo->setCurrentIndex(0);

	//Clear any information left in the form.
	m_addressEdit->clear();
	m_descriptionEdit->clear();
	m_noteEdit->clear();

	//Every newly created work order begins in the unschedueld state and will initially appear in the deck/
	m_statusCombo->setCurrentText("Unscheduled");

	//Preventing the user from manually selecting another status whiel the brand new form is being created.
	m_statusCombo->setEnabled(false);
}

//function that loads an existing work order in to the form so it can be edited.
void CreateWorkOrderScreen::editWorkOrder(int workOrderId){

	//Create an empty work order object that will receive the information retrieved from the database.
	WorkOrder order;

	//Attempt to retrieved the requested work order. Dispalay an error if it can not be loaded.
	if(!m_database->workOrderById(workOrderId, order)){
		QMessageBox::warning(this, "Work Order Error", m_database->lastError());
		return;
	}

	//Store the ID so saveWorkOrder() knows this is an edit operation.
	m_editingWorkOrderId = workOrderId;

	//Change the title and button text to indicate editing mode.
	m_titleLabel->setText(QString("Edit Work Order #%1").arg(workOrderId));
	m_saveButton->setText("Save Changes");

	//Refresh the customer list before selecting the customer associated with this work order.
	refreshCustomers();

	//Find the customer assoiciated with the work order.
	const int customerIndex = m_customerCombo->findData(order.customerId());

	//Select the correct customer if they exinst in the dropdown.
	if(customerIndex >= 0){ m_customerCombo->setCurrentIndex(customerIndex); }

	//Load the existing work-order information into the form.
	m_addressEdit->setText(order.serviceAddress());
	m_descriptionEdit->setText(order.serviceDescription());
	m_noteEdit->setPlainText(order.notes());
	m_statusCombo->setCurrentText(WorkOrder::statusToString(order.status()));
	m_statusCombo->setEnabled(true);
}

//A Function that is called whenever the selected customer changes.
void CreateWorkOrderScreen::customerChanged(){
	//Do not automatically replace the service address when editing an existing work order.
	if(m_editingWorkOrderId != 0){ return; }

	//Retrieve the selected customer's ID from the dropdown. A value of 0 means no valid customer is selected.
	const int customerId = m_customerCombo->currentData().toInt();
	if(customerId <= 0){ return; }

	//Create a customer object to hold the  database result.
	Customer customer;
	//If the selected customer exists, automatically copy their service address into the work-order address field
	if(m_database->customerById(customerId, customer)){
		m_addressEdit->setText(customer.serviceAddress());
	}
}

//Function that saves the newly created work order.
void CreateWorkOrderScreen::saveWorkOrder(){
	//Retrieve the selected customer ID
	const int customerId = m_customerCombo->currentData().toInt();

	//Retrieve and clean up the entered text. trimmed() removes extra spaces from the beginning and the end.
	const QString address = m_addressEdit->text().trimmed();
	const QString description = m_descriptionEdit->text().trimmed();

	//Validate the rquired fields a work order must have a customer, service address and service description before it can be saved.
	if(customerId <= 0 || address.isEmpty() || description.isEmpty()){
		QMessageBox::warning(this, "Invalid Work Order", "A customer, service address, and service description are required.");
		//Stop here so invalid infomration is not written to the database.
		return;
	}

	//Create a WorkOrder object.
	WorkOrder order;

	//If an existing work order is being edited, load its current data before changing any values.
	if(m_editingWorkOrderId != 0){
		if(!m_database->workOrderById(m_editingWorkOrderId, order)){
			QMessageBox::critical(this, "Database Error", m_database->lastError());
			return;
		}
	}

	//Update the work order object using the information from the form.
	order.setCustomerId(customerId);
	order.setServiceAddress(address);
	order.setServiceDescription(description);
	//Retrieve the notes form the QTextEdit field.
	order.setNotes(m_noteEdit->toPlainText().trimmed());

	//Handle creation of a brand-new work order.
	if(m_editingWorkOrderId == 0){
		//All new work orders beign as unscheduled
		order.setStatus(WorkOrder::Status::Unscheduled);
		order.setTechnicianId(std::nullopt);
		order.setScheduledStart(QDateTime());
	} else {
		const WorkOrder::Status newStatus = WorkOrder::statusFromString(m_statusCombo->currentText());
		order.setStatus(newStatus);

        if (newStatus == WorkOrder::Status::Unscheduled) {
            order.setTechnicianId(std::nullopt);
            order.setScheduledStart(QDateTime());
        }
	}

	// Add a new record when creating a work order, or update the existing database record when editing.
    const bool ok = m_editingWorkOrderId == 0 ? m_database->addWorkOrder(order) : m_database->updateWorkOrder(order);
    //If the database operation fails, display the database error instead of allowing the program to crash
    if (!ok) {
        QMessageBox::critical(this, "Database Error", m_database->lastError());
        return;
    }

    // Tell the user that the work order was saved successfully.
    QMessageBox::information(this, "Saved", "Work order was saved successfully.");

    //Notify other screens that work-order data has changed. For example, the Schedule screen can refresh its Deck.
    emit workOrderSaved();

    // Reset the form so it is ready for another new work order.
    beginNewWorkOrder();
}