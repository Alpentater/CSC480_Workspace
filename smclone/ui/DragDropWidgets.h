#pragma once

#include <QListWidget>
#include <QTableWidget>

class DeckListWidget : public QListWidget {
	Q_OBJECT

public:
	explicit DeckListWidget(QWidget *parent = nullptr);

signals:
	void workOrderReturnedToDeck(int workOrderId);

protected:
	void startDrag(Qt::DropActions supportedActions) override;
	void dragEnterEvent(QDragEnterEvent *event) override;
	void dragMoveEvent(QDragMoveEvent *event) override;
	void dropEvent(QDropEvent *event) override;
};

class ScheduleTableWidget : public QTableWidget{
	Q_OBJECT

public:
	explicit ScheduleTableWidget(QWidget *parent = nullptr);

signals:
	void workOrderDropped(int workOrderId, int row, int column);

protected:
	void startDrag(Qt::DropActions supportedActions) override;
	void dragEnterEvent(QDragEnterEvent *event) override;
	void dragMoveEvent(QDragMoveEvent *event) override;
	void dropEvent(QDropEvent *event) override;
};