//Header file for the ViewAccountScreen class.
#include "ViewAccountScreen.h"

//Qt classes used to build the account lookup interface.
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

//Provides access to customer data stored in the SQLite database.
#include "database/DatabaseManager.h"

//Constructor for the ViewAccountScreen.
ViewAccountScreen::ViewAccountScreen(DatabaseManager *database, QWidget *parent) : QWidget(parent), m_database(database){

	//Create the title shown at the top of the screen.
	auto *title = new QLabel("Accounts");

	//Increase the title font size and make it bold.
	QFont titleFont = title->font();
	titleFont.setPointSize(18);
	titleFont.setBold(true);
	title->setFont(titleFont);

	//Create the account search field and the helper text when the field is empty.
	m_searchEdit=new QLineEdit;
	m_searchEdit->setPlaceholderText("Search accounts...");

	//Create the table used to display customer accounts.
	m_table=new QTableWidget;

	//Five columns for the main customer fields.
	m_table->setColumnCount(5);

	//Label each table column.
	m_table->setHorizontalHeaderLabels({"ID", "Name", "Phone", "Email", "Service Address"});

	//Selecting a cell selects the entire customer row.
	m_table->setSelectionBehavior(QAbstractItemView::SelectRows);

	//Only allow one customer tow to be selected at a time.
	m_table->setSelectionMode(QAbstractItemView::SingleSelection);

	//Prevent users from directly editng table cells.
	m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);

	//Stretch all columns so they use the available width.
	m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	//Create the button used to edit the selected account.
	auto *editButton = new QPushButton("Edit Selected Account");

	//Create the vertical layout for this screen.
	auto *layout = new QVBoxLayout(this);

	//Add the title, search field, table, and edit button. keeping it aligned with the left.
	layout->addWidget(title);
	layout->addWidget(m_searchEdit);
	layout->addWidget(m_table);
	layout->addWidget(editButton, 0, Qt::AlignLeft);

	//Apply the search filter whenever the user types.
	connect(m_searchEdit, &QLineEdit::textChanged, this, &ViewAccountScreen::applyFilter);

	//Open the selected customer when the edit button is clicked.
	connect(editButton, &QPushButton::clicked, this, &ViewAccountScreen::editSelectedCustomer);

	//Double click any customer row also opens it for editing.
	connect(m_table, &QTableWidget::cellDoubleClicked, this, [this](int,int){editSelectedCustomer(); });
}

//Function that reloads all customer accounts from the database.
void ViewAccountScreen::refresh(){
	//Getting all the customers from the database and create one table row per customer.
	const auto customers = m_database->customers();
	m_table->setRowCount(static_cast<int>(customers.size()));

	//Add each customer to the table, adding in all their information to the table...
	int row=0;
	for(const Customer &customer : customers){
		m_table->setItem(row, 0, new QTableWidgetItem(QString::number(customer.id())));
		m_table->setItem(row, 1, new QTableWidgetItem(customer.name()));
		m_table->setItem(row, 2, new QTableWidgetItem(customer.phone()));
		m_table->setItem(row, 3, new QTableWidgetItem(customer.email()));
		m_table->setItem(row, 4, new QTableWidgetItem(customer.serviceAddress()));
		++row;
	}
	//Reapply the current search filter after refreshing. This prevents hidden rows from suddenly becoming visible if the...
	//...list is refreshed while the user is searching.
	applyFilter(m_searchEdit->text());
}

//Filters the account table using the text entered in the search field.
void ViewAccountScreen::applyFilter(const QString &text){
	//Removing unecesasry spacing first.
	const QString needle = text.trimmed();

	//Check every customer row.
	for(int row = 0; row < m_table->rowCount(); ++row){
		
		//If the search field is empty, every customer should remain visible.
		bool match = needle.isEmpty();
		
		//Search through every column in this row until a matching value is found.
		for(int column=0; column < m_table->columnCount() && !match; ++column){
			
			//Retrieve the table item in this cell.
			const auto *item = m_table->item(row, column);

			//Check whether the cell contains the search text.
			//CaseInsensitive allows things like "hElLo" to match with "hello"
			if(item && item->text().contains(needle, Qt::CaseInsensitive)) {
				match = true;
			}
		}
		//Hide the rows if none of its fields matched the search.
		m_table->setRowHidden(row, !match);
	}
}

//Requests that th ecurrently selected customer be opened in the account-editing screen.
void ViewAccountScreen::editSelectedCustomer(){
	//Determine which customer row is selected.
	const int row = m_table->currentRow();

	//Do nothing if no row is selected.
	if(row <0){ return;}
	const int id = m_table->item(row,0)->text().toInt();
	emit editCustomerRequested(id);
}