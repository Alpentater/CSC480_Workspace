//Header file for the CreateAccountScreen class.
#include "CreateAccountScreen.h"

//Qt layout and widget classes used to build the form
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QVBoxLayout>

//Header that gives the ability to use database operations.
#include "database/DatabaseManager.h"

//Constructor for the CreateAccountScreen.
CreateAccountScreen::CreateAccountScreen(DatabaseManager *database, QWidget *parent) : QWidget(parent), m_database(database){
	//Creating the title label that is at the top of the screen.
	m_titleLabel = new QLabel("Create New Account");

	//Increase the title font size and make it bold.
	QFont titleFont = m_titleLabel->font();
	titleFont.setPointSize(18);
	titleFont.setBold(true);
	m_titleLabel->setFont(titleFont);

	//Creating the text fields the user enters the information into.
	m_nameEdit = new QLineEdit;
	m_phoneEdit = new QLineEdit;
	m_emailEdit = new QLineEdit;
	m_addressEdit = new QLineEdit;

	//Creating the button that saves the account information.
	m_saveButton = new QPushButton("Save Account");

	//Using regex to validate correct phone number inputs.
	auto *phoneValidator = new QRegularExpressionValidator(QRegularExpression(R"([0-9()+\-\.\s]{0,25})"), m_phoneEdit);
	m_phoneEdit->setValidator(phoneValidator);

	//Create a form layout that places labels beside their input fields. '*' means the field is required.
	auto *form = new QFormLayout;
	form->addRow("Name*", m_nameEdit);
	form->addRow("Phone", m_phoneEdit);
	form->addRow("Email", m_emailEdit);
	form->addRow("Service Address*", m_addressEdit);

	//Add the title, customer form, and save button to the screen.
	auto *layout = new QVBoxLayout(this);
	layout->addWidget(m_titleLabel);
	layout->addLayout(form);
	layout->addWidget(m_saveButton, 0, Qt::AlignLeft);

	//Pushing the contents of the form to the top.
	layout->addStretch();

	//When the save account button is clicked, call the saveCustomer() function.
	connect(m_saveButton, &QPushButton::clicked, this, CreateAccountScreen::saveCustomer);
}

//Prepares the screen for creating a brand-new customer.
void CreateAccountScreen::beginNewCustomer(){
	//An ID of 0 means that a new customer is being created rather than an existing customer being edited.
	m_editingCustomerId = 0;

	//Resetting the screen title and button text.
	m_titleLabel->setText("Create New Account");
	m_saveButton->setText("Save Account");

	//Clearing any information left in the input fields.
	m_nameEdit->clear();
	m_phoneEdit->clear();
	m_emailEdit->clear();
	m_addressEdit->clear();

	//Place the text curston in the name field so the user can start typing right away.
	m_nameEdit->setFocus();
}

//Saves the customer information currently entered in the form.
//If m_editingCustomerId is 0, a new customer is created... otherwise, the existing customer record is updated.
void CreateAccountScreen::editCustomer(int customerId){
	//Retrieving and trimming the required customer fields.
	const QString name = m_nameEdit->text().trimmed();
	const QString address = m_addressEdit->text().trimmed();

	//Checking to make sure the required fields are filled in. If not, throw a warning and return.
	if(name.isEmpty() || address.isEmpty()){
		QMessageBox::warning(this, "Invalid Customer", "Name and service address are required.");
		return;
	}

	//If the required field check passes... create a customer object using the values that were entered in the form input fields.
	Customer customer(m_editingCustomerId, name, m_phoneEdit->text().trimmed(), m_emailEdit->text().trimmed(), address);

	//If no customer ID exists yet, add a new database record. Otherwise, update the existing customer.
	const bool ok = m_editingCustomerId == 0 ? m_database->addCustomer(customer) : m_database->updateCustomer(customer);

	//If the database operation failed, display the database error and leave the entered information on screen.
	if(!ok){
		QMessageBox::critical(this, "Database Error", m_database->lastError());
		return;
	}
	
	//Letting the user that the customer was saved successfully.
	QMessageBox::information(this, "Saved", "Customer information was saved successfully.");
	emit customerSaved();

	//Reset the form so it is ready for another new customer.
	beginNewCustomer();
}


