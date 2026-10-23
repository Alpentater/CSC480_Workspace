#pragma once

#include <QWidget>

class DatabaseManager;
class QLineEdit;
class QTableWidget;

class ViewAccountScreen : public QWidget {
	Q_OBJECT

public:
	explicit ViewAccountScreen(DatabaseManager *database, QWidget *parent = nullptr);
	void refresh();

signals:
	void editCustomerRequested(int customerId);

private slots:
	void applyFilter(const QString &text);
	void editSelectedCustomer();

private:
	DatabaseManager *m_database;
	QLineEdit *m_searchEdit;
	QTableWidget *m_table;
};
