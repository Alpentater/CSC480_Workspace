#pragma once

#include <QWidget>

class DatabaseManager;
class QLabel;
class QLineEdit;
class QPushButton;

class CreateAccountScreen : public QWidget{
	Q_OBJECT

public:
	explicit CreateAccountScreen(DatabaseManager *database, QWidget *parent = nullptr);

	void beginNewCustomer();
	void editCustomer(int customerId);

signals:
	void customerSaved();

private slots:
	void saveCustomer();

private:
	DatabaseManager *m_database;
	int m_editingCustomerId = 0;

	QLabel *m_titleLabel;
	QLineEdit *m_nameEdit;
	QLineEdit *m_phoneEdit;
	QLineEdit *m_emailEdit;
	QLineEdit *m_addressEdit;
	QPushButton *m_saveButton;
};