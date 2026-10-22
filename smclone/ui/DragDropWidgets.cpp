//Header file for the custom drag-and-drop widgets used by the schedule.
#include "DragDropWidget.h"

//Qt classes required for drag-and-drop behavior.
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>

//Anonymous namespace keeps these helper items private to this source file.
namespace {
	//Custom MIME type used to identify dragged work orders. the work order ID will be stored inside this MIME data.
	constexpr auto MimeType = "application/x-capstone-workorder-id";

	//Extracts a workorder ID will be stored inside this MIME data.
	int workOrderIdFromMime(const QMimeData *mime){
		//Used to determine whether conversion to an integer succeeds.
		bool ok = false;
		//Convert the stored MIME byte data into a QSTring, then convert that string into an integer work-order ID.
		const int id = QString::fromUtf8(mime->data(MimeType)).toInt(&ok);
		//Return the ID if conversion succeeded, return 0 to represent an invalid work-order ID.
		return ok ? id : 0;
	}
}

//Constructor for the Deck list widget. The deck contains work orderss that have not yet been scheduled.
DeckListWidget::DeckListWidget(QWidget *parent) : QListWidget(parent){
	//Allow work orders in the deck to be dragged.
	setDragEnabled(true);
	//Allow scheduled work orders to be dropped back into the deck.
	setAcceptDrops(true);
	//Show the user where a dragged item can be dropped.
	setDropIndicatorShown(true);
	//Dragging and dropping should move thne work order rather than creating a copy.
	setDefaultDropAction(Qt::MoveAction);
}

//Begins dragging the currently selected work order from the deck.
void DeckListWidget::startDrag(Qt::DropActions){
	//Get the currently selected item in the Deck.
	QListWidgetItem *item = currentItem();
	if(!item){ return; } //do nothing if no work order is selected.

	//Retrive the work-order ID stored in the item's UserRole data.
	const int id = item->data(Qt::UserRole).toInt();
	//Create MIME data that will travel with the dragged item.
	auto *mime = new QMimeData;
	//Store the work order ID inside the custom MIME type.
	mime->setData(MimeType, QByteArray::number(id));

	//Create the Qt drag ooperation, attach the work order mime data to the drag, and begin the drag operation using Move Action.
	QDrag drag(this);
	drag.setMimeData(mime);
	drag.exec(Qt::MoveAction);
}

//Called when a dragged object first enters the Deck.
void DeckListWidget::dragEnterEvent(QDragEnterEvent *event){
	if(event->mimeData()->hasFormat(MimeType)) {
		event->acceptProposedAction();
	} else {
		event->ignore();
	}
}

//Called while a dragged object moves around inside the Deck.
void DeckListWidget::dragMoveEvent(QDragMoveEvent *event){
	if(event->mimeData()->hasFormat(MimeType)){
		event->acceptProposedAction();
	} else {
		event->ignore();
	}
}

//Called when a work order is dropped into the Deck.
void DeckListWidget::dropEvent(QDropEvent *event){
	if(!event->mimeData()->hasFormat(MimeType)){
		event->ignore();
		return;
	}

	const int id = workOrderIdFromMime(event->mimeData());
	if(id > 0){
		emit workOrderReturnedToDeck(id);
		event->acceptProposedAction();
	} else {
		event -> ignore();
	}
}

//Constructor for the main schedule table. This table allows work orders to be dragged between technician columns and time slots.
ScheduleTableWidget::ScheduleTableWidget(QWidget *parent):QTableWidget(parent){
	setDragEnabled(true);
	setAcceptDrops(true);
	setDropIndicatorShown(true);
	setDragDropMode(QAbstractItemView::DragDrop);
	setDefaultDropAction(Qt::MoveAction);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setSelectionBehavior(QAbstractItemView::SelectItems);
}

//Begins dragging a work order that is already on the schedule.
void ScheduleTableWidget::startDrag(Qt::DropActions){
	QTableWidgetItem *item = currentItem();
	if(!item){ return; }

	const int id = item->data(Qt::UserRole).toInt();
	if(id<= 0){ return; }

	auto *mime = new QMimeData;
	mime->setData(MimeType, QByteArray::number(id));

	QDrag drag(this);
	drag.setMimeData(mime);
	drag.exec(Qt::MoveAction);
}

//Called when a dragged work order first enters the schedule.
void ScheduleTableWidget::dragEnterEvent(QDragEnterEvent *event){
	if(event->mimeData()->hasFormat(MimeType)){
		event->acceptProposedAction();
	} else {
		event->ignore();
	}
}

//Called as a dragged work order moves around the schedule.
void ScheduleTableWidget::dragMoveEvent(QDragMoveEvent *event){
	if(event->mimeData()->hasFormat(MimeType)) { 
		event->acceptProposedAction();
	} else { 
		event->ignore();
	}
}

//Called when a work order is dropped onto the schedule.
void ScheduleTableWidget::dropEvent(QDropEvent *event){
	if(!event->mimeData()->hasFormat(MimeType)){
		event->ignore();
		return;
	}

	const QModelIndex target = indexAt(event->position().toPoint());
	if(!target.isValid()){
		event->ignore();
		return;
	}

	const int id = workOrderIdFromMime(event->mimeData());
	if(id > 0){
		emit workOrderDropped(id, target.row(), target.column());
		event->acceptProposedAction();
	} else {
		event->ignore();
	}
}