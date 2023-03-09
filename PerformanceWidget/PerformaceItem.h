#pragma once

#include <QWidget>
#include "ui_PerformaceItem.h"

class PerformaceItem : public QWidget
{
	Q_OBJECT

public:
	PerformaceItem(int status,QString value,QWidget *parent = nullptr);
	~PerformaceItem();

private:
	Ui::PerformaceItemClass ui;
};
