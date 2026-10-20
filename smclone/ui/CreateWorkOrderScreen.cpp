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

	m_customerCombo = new QComboBox;
	m_addressEdit = new QLineEdit;
	m_descriptionEdit = new QLineEdit;
	m_notesEdit = new QTextEdit;
	m_notesEdit->setMaximumHeight(130);
	m_statusCombo = newQComboBox;
	m_statusCombo->addItems({"Unscheduled", "Scheduled", "In Progress", "Completed", "Cancelled"});
	m_saveButton = new QPushButton("Save Work Order");

	auto *form = new QFormLayout;
	form->addRow("Customer*", m_customerCombo);
	form->addRow("Service Address*", m_addressEdit);
	form->addRow("Service Description", m_descriptionEdit);
	form->addRow("Notes", m_notesEdit);
	form->addRow("Status", m_statusCombo);

	auto *layout = new QVBoxLayout(this);
	layout->addWidget(m_titleLabel);
	layout->addLayout(form);
	layout->addWidget(m_saveButton, 0, Qt::AlignLeft);
	layout->AddStretch();

	connect(m_saveButton, &QPushButton::clicked, this, &CreateWorkOrderScreen::saveWorkOrder);
	connect(m_customerCombo, &QComboBox::currentIndexChanged, this, &CreateWorkOrderScreen::customerChanged);

	refreshCustomers();
	beginNewWorkOrder();
}

void CreateWorkOrderScreen::refreshCustomers(){
	const int previousId = m_customerCombo->currentData().toInt();
	m_customerCombo->blockSignals(true);
	m_customerCombo->clear();
	m_customerCombo->addItem("Select a customer...", 0);

	for(const Customer &customer : m_database->customers()){
		m_customerCombo->addItem(customer.name(), customer.id());
	}

	const int restored = m_customerCombo->findData(previousId);
	if(restored >= 0) { m _customerCombo->setCurrentIndex(restored); }
	m_customerCombo->blockSignals(false);
}

void CreateWorkOrderScreen::beginNewWorkOrder(){
	m_editingWorkOrderId = 0;
	m_titleLabel->setText("Create New Work Order");
	m_saveButton->setText("Save Work Order");
	refreshCustomers();
	m_customerCombo->setCurrentIndex(0);
	m_addressEdit->clear();
	m_descriptionEdit->clear();
	m_notesEdit->clear();
	m_statusCombo->setCurrentText("Unscheduled");
	m_statusCombo->setEnabled(false);
}

void CreateWorkOrderScreen::editWorkOrder(int workOrderId){
	WorkOrder order;
	if(!m_database->workOrderById(workOrderId, order)){
		QMessageBox::warning(this, "Work Order Error", m_database->lastError());
		return;
	}

	m_editingWorkOrderId = workOrderId;
	m_titleLabel->setText(QString("Edit Work Order #%1").arg(workOrderId));
	m_saveButton->setText("Save Changes");
	refreshCustomers();

	const int customerIndex = m_customerCombo->findData(order.customerId());
	if(customerIndex >= 0){
		m_customerCombo->setCurrentIndex(customerIndex);
	}

	m_addressEdit->setText(order.serviceAddress());
	m_descriptionEdit->setText(order.serviceDescription());
	m_notesEdit->setPlainText(order.notes());
	m_statusCombo->setCurrent(WorkOrder::statusToString(order.status()));
	m_statusCombo->setEnabled(true);
}

void CreateWorkOrderScreen::customerChanged(){
	if(m_editingWorkOrderId != 0){ return; }

	const int customerId = m_customerCombo->currentData().toInt();
	if(customerId <= 0){ return; }

	Customer customer;
	if(m_database->customerById(customerId, customer)){
		m_addressEdit->setText(customer.serviceAddress());
	}
}

void CreateWorkOrderScreen::saveWorkOrder(){
	const int customerId = m_customerCombo->currentData().toInt();
	const QString address = m_adddressEdit->text().trimmed();
	const QString description = m_descriptionEdit->text().trimmed();

	if(customerId <= 0 || address.isEmpty() || description.isEmpty()){
		QMessageBox::warning(this, "Invalid Work Order", "A customer, service address, and service description are required.");
		return;
	}

	WorkOrder order;
	if(m_editingWorkOrderId != 0){
		if(!m_database->workOrderById(m_editingWorkOrderId, order)){
			QMessageBox::critical(this, "Database Error", m_database->lastError());
			return;
		}
	}

	order.setCustomerId(customerId);
	order.setServiceAddress(address);
	order.setServiceDescription(description);
	order.setNotes(m_notesEdit->toPlainText().trimmed());

	if(m_editingWorkOrderId == 0){
		order.setStatus(WorkOrder::Status::Unscheduled);
		order.setTechnicianId(std::nullopt);
		order.setScheduledStart(QDateTime());
	} else {
		const WorkOrder::Status newStatus = WorkOrder::statusFromString(m_statusCombo->currentText());
		order.setStatus(newStatus);

		// Returning an existing work order to Unscheduled also clears its
        // technician and appointment so it correctly returns to the Deck.
        if (newStatus == WorkOrder::Status::Unscheduled) {
            order.setTechnicianId(std::nullopt);
            order.setScheduledStart(QDateTime());
        }
	}

    const bool ok = m_editingWorkOrderId == 0 ? m_database->addWorkOrder(order) : m_database->updateWorkOrder(order);
    if (!ok) {
        QMessageBox::critical(this, "Database Error", m_database->lastError());
        return;
    }

    QMessageBox::information(this, "Saved", "Work order was saved successfully.");
    emit workOrderSaved();
    beginNewWorkOrder();
}