#pragma once

#include <QWidget>

class DatabaseManager;
class QLineEdit;
class QTableWidget;

class ViewWorkOrderScreen : public QWidget {
	Q_OBJECT

public:
	explicit ViewWorkOrderScreen(DatabaseManager *database, QWidget *parent = nullptr);
	void refresh();

signals:
	void editWorkOrderRequested(int workOrderId);

private slots:
	void applyFilter(const QString &text);
	void editSelectedWorkOrder();

private:
	DatabaseManager *m_database;
	QLineEdit *m_searchEdit;
	QTableWidget *m_table;
};