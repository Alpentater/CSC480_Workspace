#pragma once

#include <QMainWindow>

class DatabaseManager;
class QStackedWidget;
class CreateAccountScreen;
class ViewAccountScreen;
class CreateWorkOrderScreen;
class ViewWorkOrderScreen;
class ScheduleScreen;

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	explicit MainWindow(DatabaseManager *database, QWidget *parent = nullptr);

private slots:
	void showCreateAccount();
	void showAccounts();
	void showCreateWorkOrder();
	void showWorkOrders();
	void showScheduleToday();
	void chooseScheduleDate();
	void editCustomer(int customerId);
	void editWorkOrder(int workOrderId);
	void handleCustomerSaved();
	void handleWorkOrderSaved();

private:
	DatabaseManager *m_database;
	QStackedWidget *m_stack;

	CreateAccountScreen *m_createAccountScreen;
	ViewAccountScreen *m_viewAccountScreen;
	CreateWorkOrderScreen *m_createWorkOrderScreen;
	ViewWorkOrderScreen *m_viewWorkOrderScreen;
	ScheduleScreen *m_scheduleScreen;
}