#include "ViewWorkOrderScreen.h"

#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include "database/DatabaseManager.h"

ViewWorkOrderScreen::ViewWorkOrderScreen(DatabaseManager *database, QWidget *parent) : QWidget(parent), m_database(database) {
    auto *title = new QLabel("Work Orders");
    QFont titleFont = title->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    title->setFont(titleFont);

    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search work orders...");

    m_table = new QTableWidget;
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels({"ID", "Customer", "Address", "Service", "Status", "Scheduled", "Technician"});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    auto *editButton = new QPushButton("Edit Selected Work Order");

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(title);
    layout->addWidget(m_searchEdit);
    layout->addWidget(m_table);
    layout->addWidget(editButton, 0, Qt::AlignLeft);

    connect(m_searchEdit, &QLineEdit::textChanged, this, &ViewWorkOrderScreen::applyFilter);
    connect(editButton, &QPushButton::clicked, this, &ViewWorkOrderScreen::editSelectedWorkOrder);
    connect(m_table, &QTableWidget::cellDoubleClicked, this, [this](int, int) { editSelectedWorkOrder(); });
}

void ViewWorkOrderScreen::refresh(){
    const auto orders = m_database->allWorkOrders();
    const auto techs = m_database->technicians();

    m_table->setRowCount(static_cast<int>(orders.size()));

    int row = 0;
    for (const WorkOrder &order : orders) {
        Customer customer;
        QString customerName = QString("Customer #%1").arg(order.customerId());
        if (m_database->customerById(order.customerId(), customer)) {
            customerName = customer.name();
        }

        QString technicianName = "Unassigned";
        if (order.technicianId()) {
            for (const Technician &tech : techs) {
                if (tech.id() == *order.technicianId()) {
                    technicianName = tech.name();
                    break;
                }
            }
        }

        m_table->setItem(row, 0, new QTableWidgetItem(QString::number(order.id())));
        m_table->setItem(row, 1, new QTableWidgetItem(customerName));
        m_table->setItem(row, 2, new QTableWidgetItem(order.serviceAddress()));
        m_table->setItem(row, 3, new QTableWidgetItem(order.serviceDescription()));
        m_table->setItem(row, 4, new QTableWidgetItem(WorkOrder::statusToString(order.status())));
        m_table->setItem(row, 5, new QTableWidgetItem(order.scheduledStart().isValid() ? order.scheduledStart().toString("yyyy-MM-dd h:mm AP") : "—"));
        m_table->setItem(row, 6, new QTableWidgetItem(technicianName));
        ++row;
    }

    applyFilter(m_searchEdit->text());
}

void ViewWorkOrderScreen::applyFilter(const QString &text){
    const QString needle = text.trimmed();
    for (int row = 0; row < m_table->rowCount(); ++row) {
        bool match = needle.isEmpty();
        for (int column = 0; column < m_table->columnCount() && !match; ++column) {
            const auto *item = m_table->item(row, column);
            if (item && item->text().contains(needle, Qt::CaseInsensitive)){ match = true;}
        }
        m_table->setRowHidden(row, !match);
    }
}

void ViewWorkOrderScreen::editSelectedWorkOrder() {
    const int row = m_table->currentRow();
    if (row < 0) return;
    emit editWorkOrderRequested(m_table->item(row, 0)->text().toInt());
}
