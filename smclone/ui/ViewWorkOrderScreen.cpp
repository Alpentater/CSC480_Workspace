//Header file for the ViewWorkOrderScreen class.
#include "ViewWorkOrderScreen.h"

//Qt classes used to build the work-order lookup interface.
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

//Provides access to work orders, customers, and technicians stored in the SQLite database.
#include "database/DatabaseManager.h"

//Constructor for the ViewWorkOrderScreen. This screen allows the user to...
//...view all work orders, search/filter the list, select a work order, and open a work order for editing.
ViewWorkOrderScreen::ViewWorkOrderScreen(DatabaseManager *database, QWidget *parent) : QWidget(parent), m_database(database) {

    //Create the title displayed at the top of the screen.
    auto *title = new QLabel("Work Orders");

    //Increase the title font size and make it bold.
    QFont titleFont = title->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    title->setFont(titleFont);

    //Create the search field used to filter work orders.
    m_searchEdit = new QLineEdit;

    //Display helper text when the search box is empty.
    m_searchEdit->setPlaceholderText(
        "Search work orders..."
    );

    //Create the table used to display work-order information.
    m_table = new QTableWidget;

    //The table contains seven columns.
    m_table->setColumnCount(7);

    //Set the label for each column.
    m_table->setHorizontalHeaderLabels({ "ID", "Customer", "Address", "Service", "Status", "Scheduled", "Technician" });

    //Selecting any cell selects the entire work-order row.
    m_table->setSelectionBehavior( QAbstractItemView::SelectRows);

    //Only allow one work order to be selected at a time.
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);

    //Prevent direct editing of values inside the table. Work orders should be edited through the editing screen.
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //Stretch each column to make use of the available width.
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    //Create the button used to edit the selected work order.
    auto *editButton = new QPushButton("Edit Selected Work Order");

    //Create the vertical layout for the screen.
    auto *layout = new QVBoxLayout(this);

    //Add the title, search field, table, and edit button.
    layout->addWidget(title);
    layout->addWidget(m_searchEdit);
    layout->addWidget(m_table);

    //Keep the edit button aligned to the left side.
    layout->addWidget(editButton,0, Qt::AlignLeft);

    //Update the filter whenever the user types in the search box.
    connect(m_searchEdit, &QLineEdit::textChanged, this, &ViewWorkOrderScreen::applyFilter);

    //Open the selected work order when the Edit button is clicked.
    connect(editButton, &QPushButton::clicked, this, &ViewWorkOrderScreen::editSelectedWorkOrder);

    //Double-clicking a work-order row also opens it for editing.
    connect(m_table, &QTableWidget::cellDoubleClicked, this, [this](int, int) { editSelectedWorkOrder(); });
}

//Reloads all work orders from the database and displays them in the table.
void ViewWorkOrderScreen::refresh() {
    //Retrieve every work order from the database.
    const auto orders = m_database->allWorkOrders();

    //Retrieve active technicians so technician IDs can be converted into readable technician names.
    const auto techs = m_database->technicians();

    //Create one table row for each work order.
    m_table->setRowCount(static_cast<int>(orders.size()));

    // Add each work order to the table.
    int row = 0;
    for (const WorkOrder &order : orders){
        Customer customer;
        //Use a fallback value in case the customer cannot be retrieved from the database.
        QString customerName = QString("Customer #%1").arg(order.customerId());

        //Attempt to replace the fallback with the actual customer's name.
        if (m_database->customerById(order.customerId(),customer)){
            customerName = customer.name();
        }

        //Unscheduled work orders do not have a technician.
        QString technicianName = "Unassigned";

        //Only search for a technician if one is assigned.
        if (order.technicianId()){
            //Search through the active technicians.
            for (const Technician &tech : techs) {
                //Find the technician whose ID matches the work order's assigned technician ID.
                if (tech.id() == *order.technicianId()){
                    technicianName = tech.name();
                    break;
                }
            }
        }

        //Work-order ID.
        m_table->setItem(row, 0, new QTableWidgetItem(QString::number(order.id())));

        //Customer name.
        m_table->setItem(row, 1, new QTableWidgetItem(customerName));

        //Service address.
        m_table->setItem(row, 2, new QTableWidgetItem(order.serviceAddress()));

        //Service description.
        m_table->setItem(row, 3, new QTableWidgetItem(order.serviceDescription()));

        //Current work-order state.
        m_table->setItem(row, 4, new QTableWidgetItem(WorkOrder::statusToString(order.status())));

        //Scheduled date and time and unscheduled work orders display a dash instead.
        m_table->setItem(row, 5, new QTableWidgetItem(order.scheduledStart().isValid() ? order.scheduledStart().toString("yyyy-MM-dd h:mm AP") : "—"));

        //Assigned technician.
        m_table->setItem(row, 6, new QTableWidgetItem(technicianName));

        ++row;
    }


    //Reapply the current search filter after reloading the table from the database.
    applyFilter(m_searchEdit->text());
}


//Filters the table according to the user's search text.
void ViewWorkOrderScreen::applyFilter(const QString &text) {
    //Remove extra spaces from the beginning and end of the search text.
    const QString needle = text.trimmed();

    //Check each work-order row.
    for (int row = 0; row < m_table->rowCount(); ++row) {
        //An empty search means every row should be visible.
        bool match = needle.isEmpty();

        //Search across every column in the row.
        for (int column = 0; column < m_table->columnCount() && !match; ++column){
            //Get the current table cell.
            const auto *item = m_table->item(row,column);

            //Match the search text against the cell value. CaseInsensitive means searches such as "scheduled" can match "Scheduled".
            if (item && item->text().contains(needle, Qt::CaseInsensitive)){
                match = true;
            }
        }

        // Hide the row when none of its values match.
        m_table->setRowHidden(row, !match);
    }
}


//Requests that the currently selected work order be opened in the work-order editing screen.
void ViewWorkOrderScreen::editSelectedWorkOrder(){
    //Determine which row is currently selected.
    const int row = m_table->currentRow();

    //Do nothing when no work order is selected.
    if (row < 0){ return; }

    //Retrieve the item containing the work-order ID.
    QTableWidgetItem *idItem = m_table->item(row, 0);

    //Protect against an unexpected empty table cell.
    if (!idItem){ return; }

    //Convert the ID text into an integer.
    const int id = idItem->text().toInt();

    //Request that MainWindow open this work order in the editing screen.
    if (id > 0) { emit editWorkOrderRequested(id); }
}