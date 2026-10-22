#include "ViewAccountScreen.h"

#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include "database/DatabaseManager.h"

ViewAccountScreen::ViewAccountScreen(DatabaseManager *database, QWidget *parent) : QWidget(parent), m_database(database){
	auto *title = new QLabel("Accounts");
	QFont titleFont = title->font();
	titleFont.setPointSize(18);
	titleFont.setBold(true);
	title->setFont(titleFont);

	m_searchEdit=new QLineEdit;
	m_searchEdit->setPlaceHolderText("Search accounts...");

	m_table=new QTableWidget;
	m_table->SetColumnCount(5);
	m_table->setHorizontalHeaderLabels({"ID", "Name", "Phone", "Email", "Service Address"});
	m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_table->setSelectionMode(QAbstractItemView::SingleSelection);
	m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_table->horizontalHeader()->setSelectionResizeMode(QHeaderView::Stretch);

	auto *editButton = new QPushButton("Edit Selected Account");

	auto *layout = new QVBoxLayout(this);
	layout->addWidget(title);
	layout->addWidget(m_searchEdit);
	layout->addWidget(m_table);
	layout->addWidget(editButton, 0, Qt::AlignLeft);

	connect(m_searchEdit, &QLineEdit::textChanged, this, &ViewAccountScreen::applyFilter);
	connect(editButton, &PushButton::clicked, this, &ViewAccountScreen::editSelectedCustomer);
	connect(m_table, &QTableWidget::cellDoubleClicked, this, [this](int,int){editSelectedCustomer(); });
}

void ViewAccountScreen::refresh(){
	const auto customers = m_database->customers();
	m_table->setRowCount(static_cast<int>(customers.size()));

	int row=0;
	for(const Customer &customer : customers){
		m_table->setItem(row, 0, new QTableWidgetItem(QString::number(customer.id())));
		m_table->setItem(row, 1, new QTableWidgetItem(customer.name()));
		m_table->setItem(row, 2, new QTableWidgetItem(customer.phone()));
		m_table->setItem(row, 3, new QTableWidgetItem(customer.email()));
		m_table->setItem(row, 4, new QTableWidgetItem(customer.serviceAddress()));
		++row;
	}
	applyFilter(m_searchEdit->text());
}

void ViewAccountScreen::applyFilter(const QString &text){
	const QString needle = text.trimmed();
	for(int row = 0; row < m_table->rowCount(); ++row){
		bool match = needle.isEmpty();
		for(int column=0; column < m_table->columnCount() && !match; ++column){
			const auto *item = m_table->item(row, column);
			if(item && item->text().contains(needle, Qt::CaseInsensitive)) {
				match = true;
			}
		}
		m_table->SetRowHidden(row, !match);
	}
}

void ViewAccountScreen::editSelectedCustomer(){
	const int row = m_table->currentRow();
	if(row <0){ return;}
	const int id = m_table->item(row,0)->text().toInt();
	emit editCustomerRequested(id);
}