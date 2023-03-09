#include "PerformaceItem.h"
#include <qdatetime.h>


const QString m_red_SheetStyle = "min-width: 8px; min-height: 8px;max-width:8px; max-height: 8px;border-radius: 4px;background:red";

const QString m_green_SheetStyle = "min-width: 8px; min-height: 8px;max-width:8px; max-height: 8px;border-radius: 4px;background:#45b97c";

const QString m_yellow_SheetStyle = "min-width: 8px; min-height: 8px;max-width:8px; max-height: 8px;border-radius: 4px;background:yellow";


PerformaceItem::PerformaceItem(int status, QString value,QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	setWindowFlags(Qt::FramelessWindowHint);


	QDateTime dateTime;
	dateTime = QDateTime::currentDateTime();
	ui.label_time->setText(dateTime.toString("yyyy-MM-dd hh:mm:ss"));
	ui.label_status->setText("");
	if(1 == status)
		ui.label_status->setStyleSheet(m_green_SheetStyle);
	else if (2 == status)
		ui.label_status->setStyleSheet(m_yellow_SheetStyle);
	else
		ui.label_status->setStyleSheet(m_red_SheetStyle);
	ui.label_detail->setText(value);

}

PerformaceItem::~PerformaceItem()
{}

