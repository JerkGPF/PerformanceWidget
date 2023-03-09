#pragma once

#include <QtWidgets/QWidget>
#include "ui_PerformanceWidget.h"
#include <QTimer>
#include <QMetaType>
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock.h>
#include <iphlpapi.h>
class PerformanceThread : public QObject
{
	Q_OBJECT

public:
	PerformanceThread(QObject* parent = nullptr);
	~PerformanceThread();

	// 时间转换    
	unsigned int file_time_2_utc(const FILETIME* ftime);

	// 获得CPU的核数    
	int get_processor_number();

signals:
	void signal_cpuPercentChanged(float cpuPercent);
	void signal_memoryPercentChanged(int memoryPercent);
	void signal_intnetNumChanged(DWORD intnetNum);

public slots:
	void start();

private slots:
	void slot_cpuTimeout();
	void slot_memoryTimeout();
	void slot_intnetTimeout();

private:
	QTimer* m_cpuTimer;
	QTimer* m_memoryTimer;
	QTimer* m_intnetTimer;

	float m_cpuPercent;
	int m_memoryPercent;
	int m_intnetNum;

	int _processor;
	unsigned int _last_time = 0;         //上一次的时间    
	unsigned int _last_system_time = 0;

	DWORD   dwLastIn = 0;           //上一秒钟的接收字节数
	DWORD   dwLastOut = 0;          //上一秒钟的发送字节数
	DWORD   dwBandIn = 0;           //下载速度
	DWORD   dwBandOut = 0;          //上传速度


	QString NetSpeedInfo;


};


class PerformanceWidget : public QWidget
{
	Q_OBJECT

public:
	PerformanceWidget(QWidget* parent = 0);
	~PerformanceWidget();

	QFont getIconFont();

signals:
	void signal_overCpuThreshold();

public slots:


private slots:
	void slot_cpuPercentChanged(float cpuPercent);
	void slot_memoryPercentChanged(int memoryPercent);
	void slot_intnetNumChanged(DWORD intnetNum);
	void slot_overCpuThreadholdTimeout();

private:
	void initWidget();
	void initUIStyles();
	void initText();

	void addItem(int status,QString value);
private:
	Ui::PerformanceWidget ui;

	PerformanceThread* m_pPerformanceThread;
	QThread* m_pThread;

	QTimer* m_overCpuThreadholdTimer;      //超过cpu阈值定时器
};


