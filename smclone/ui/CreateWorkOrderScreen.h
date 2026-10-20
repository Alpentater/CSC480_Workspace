#pragma once

#include <QWidget>

class DatabaseManager;
class QLabel;
class QComboBox;
class QLineEdit;
class QTextEdit;
class QPushButton;

class CreateWorkOrderScreen : public QWidget {
	Q_OBJECT

public:
	explicit CreateWorkOrderScreen(DatabaseManager *database, QWidget *parent = nullptr);

	void beginNewWorkOrder();
	void editWorkOrder(int workOrderId);
	void refreshCustomers();

signals:
	void workOrderSaved();

private slots:
	void saveWorkOrder();
	void customerChanged();

private:
	DatabaseManager *m_database;
	int m_editingWorkOrderId = 0;

	QLabel *m_titleLabel;
	QComboBox *m_customerCombo;
	QLineEdit *m_addressEdit;
	QLineEdit *m_descriptionEdit;
	QTextEdit *m_noteEdit;
	QComboBox *m_statusCombo;
	QPushButton *m_saveButton;
};