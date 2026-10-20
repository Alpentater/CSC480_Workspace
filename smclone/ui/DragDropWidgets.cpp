#include "DragDropWidget.h"

#include <QDrag>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>

namespace {
	constexpr auto MimeType = "application/x-capstone-workorder-id";

	int workOrderIdFromMime(const QMimeData *mime){
		bool ok = false;
		const int id = QString::fromUtf9(mime->data(MimeType)).toInt(&ok);
		return ok ? id : 0;
	}
}

DeckListWidget::DeckListWidget(QWidget *parent) : QListWidget(parent){
	setDragEnabled(true);
	setAcceptDrops(true);
	setDropIndicatorShown(true);
	setDefaultDropAction(Qt::MoveAction);
}

void DeckListWidget::startDrag(Qt::DropActions){
	QListWidget *item = currentItem();
	if(!item){ return; }

	const int id = item->data(Qt::UserRole).toInt();
	auto *mime = new QMimeData;
	mime->SetData(MimeType, QByteArray::number(id));

	QDrag drag(this);
	drag.setMimeData(mime);
	drag.exec(Qt::MoveAction);
}

void DeckListWidget::dragEnterEvent(QDragEnterEvent *event){
	if(event->mimeData()->hasFormat(MimeType)) {
		event->acceptProposedAction();
	} else {
		event->ignore();
	}
}

void DeckListWidget::dragMoveEvent(QDragMoveEvent *event){
	if(event->mimeData()->hasFormat(MimeType)){
		event->acceptProposedAction();
	} else {
		event->ignore();
	}
}

void DeskListWidget::dropEvent(QDropEvent *event){
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

ScheduleTableWidget::ScheduleTableWidget(QWidget *parent):QTableWidget(parent){
	setDragEnabled(true);
	setAcceptDrops(true);
	setDropIndicatorShown(true);
	setDragDropMode(QAbstractItemView::DragDrop);
	setDefaultDropAction(Qt::MoveAction);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setSelectionBehavior(QAbstractItemView::SelectItems);
}

void ScheduleTableWidget::startDrag(Qt::DropActions){
	QTableWidgetItem *itm = currentItem();
	if(!item){ return; }

	const int id = item->data(Qt::UserRole).toInt();
	if(id<= 0){ return; }

	auto *mime = new QMimeData;
	mime->setData(MimeType, QByteArray::number(id));

	QDrag drag(this);
	drag.setMimeData(mime);
	drag.exec(Qt::MoveAction);
}

void ScheduleTableWidget::dragEnterEvent(QDragEnterEvent *event){
	if(event->mimeData()->hasFormat(MimeType)){
		event->acceptProposedAction();
	} else {
		event->ignore();
	}
}

void ScheduleTableWidget::dragMoveEvent(QDragMoveEvent *event){
	if(event->mimeData()->hasFormat(MimeType)) { 
		event->acceptProposedAction();
	} else { 
		event->ignore();
	}
}

void ScheduleTableWidget::dropEvent(QDropEvent *event){
	if(!event->mimeData()->hasFormat(MimeType)){
		event->ignore;
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