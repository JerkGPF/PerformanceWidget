#include "PerformanceWidget.h"
#include "IconFontString.h"
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextCodec>
#include <QDebug>
#include <QFontDatabase>
#include "PerformaceItem.h"

#define CPU_DETECTION_INTERVAL  1000
#define MEMORY_DETECTION_INTERVAL 1000
#define INTNET_DETECTION_INTERVAL 1000

#pragma comment(lib, "IPHLPAPI.lib")


#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

PerformanceThread::PerformanceThread(QObject* parent)
	:QObject(parent)
{
	m_cpuPercent = -1;
	m_memoryPercent = -1;
	m_intnetNum = -1;
	_processor = get_processor_number();


}

PerformanceThread::~PerformanceThread()
{
	//delete[] m_pTable;
}

void PerformanceThread::start()
{
	m_cpuTimer = new QTimer;
	m_intnetTimer = new QTimer;
	m_memoryTimer = new QTimer;
	connect(m_cpuTimer, SIGNAL(timeout()), this, SLOT(slot_cpuTimeout()));
	connect(m_intnetTimer, SIGNAL(timeout()), this, SLOT(slot_intnetTimeout()));
	connect(m_memoryTimer, SIGNAL(timeout()), this, SLOT(slot_memoryTimeout()));

	m_cpuTimer->start(CPU_DETECTION_INTERVAL);
	m_memoryTimer->start(MEMORY_DETECTION_INTERVAL);
	m_intnetTimer->start(INTNET_DETECTION_INTERVAL);
}


void PerformanceThread::slot_cpuTimeout()
{
	if (m_cpuTimer->isActive())
	{
		m_cpuTimer->stop();
	}
#ifdef Q_OS_WIN
	/*static FILETIME preidleTime;
	static FILETIME prekernelTime;
	static FILETIME preuserTime;

	FILETIME idleTime;
	FILETIME kernelTime;
	FILETIME userTime;

	GetSystemTimes(&idleTime, &kernelTime, &userTime);

	quint64 a, b;
	long idle, kernel, user;

	a = (preidleTime.dwHighDateTime << 31) | preidleTime.dwLowDateTime;
	b = (idleTime.dwHighDateTime << 31) | idleTime.dwLowDateTime;
	idle = b - a;

	a = (prekernelTime.dwHighDateTime << 31) | prekernelTime.dwLowDateTime;
	b = (kernelTime.dwHighDateTime << 31) | kernelTime.dwLowDateTime;
	kernel = b - a;

	a = (preuserTime.dwHighDateTime << 31) | preuserTime.dwLowDateTime;
	b = (userTime.dwHighDateTime << 31) | userTime.dwLowDateTime;
	user = b - a;

	m_cpuPercent = (kernel + user - idle) * 1.0 / (kernel + user) * 100;

	long cc = kernel + user;
	cc = cc - idle;
	cc = cc * 100;
	cc = cc / (kernel + user);

	preidleTime = idleTime;
	prekernelTime = kernelTime;
	preuserTime = userTime;
	*/

	FILETIME now;
	FILETIME creation_time;
	FILETIME exit_time;
	FILETIME kernel_time;
	FILETIME user_time;
	unsigned int system_time;
	unsigned int time;
	unsigned int system_time_delta;
	unsigned int time_delta;
	GetSystemTimeAsFileTime(&now);

	HANDLE _hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, GetCurrentProcessId());

	//计算占用CPU的百分比  
	if (!GetProcessTimes(_hProcess, &creation_time, &exit_time, &kernel_time, &user_time))
	{
		m_cpuTimer->start(CPU_DETECTION_INTERVAL);
		return;
	}
	system_time = (file_time_2_utc(&kernel_time) + file_time_2_utc(&user_time)) / _processor;
	time = file_time_2_utc(&now);

	//判断是否为首次计算  
	if ((_last_system_time == 0) || (_last_time == 0))
	{
		_last_system_time = system_time;
		_last_time = time;
	}
	else
	{
		system_time_delta = system_time - _last_system_time;
		time_delta = time - _last_time;

		if (time_delta == 0)
		{
			m_cpuTimer->start(CPU_DETECTION_INTERVAL);
			return;
		}
		m_cpuPercent = (float)system_time_delta * 100 / (float)time_delta;
		_last_system_time = system_time;
		_last_time = time;
		emit signal_cpuPercentChanged(m_cpuPercent);
	}
#endif
	m_cpuTimer->start(CPU_DETECTION_INTERVAL);
}

unsigned int PerformanceThread::file_time_2_utc(const FILETIME* ftime)
{
	LARGE_INTEGER li;

	li.LowPart = ftime->dwLowDateTime;
	li.HighPart = ftime->dwHighDateTime;
	return li.QuadPart;
}

int PerformanceThread::get_processor_number()
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return info.dwNumberOfProcessors;
}

void PerformanceThread::slot_memoryTimeout()
{
	if (m_memoryTimer->isActive())
	{
		m_memoryTimer->stop();
	}

#ifdef Q_OS_WIN
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);
	m_memoryPercent = statex.dwMemoryLoad;

	emit signal_memoryPercentChanged(m_memoryPercent);
#endif
	m_memoryTimer->start(MEMORY_DETECTION_INTERVAL);
}

void PerformanceThread::slot_intnetTimeout()
{
	if (m_intnetTimer->isActive())
	{
		m_intnetTimer->stop();
	}

	NetSpeedInfo.clear();

	//m_pTable = (MIB_IFTABLE *)MALLOC(sizeof (MIB_IFTABLE));
	MIB_IFTABLE* m_pTable = NULL;
	DWORD    m_dwAdapters = 0;
	if (GetIfTable(NULL, &m_dwAdapters, FALSE) == ERROR_INSUFFICIENT_BUFFER)
	{
		m_pTable = (MIB_IFTABLE*)MALLOC(m_dwAdapters);
	}
	if (GetIfTable(m_pTable, &m_dwAdapters, FALSE) == NO_ERROR)
	{
		DWORD   dwInOctets = 0;
		DWORD   dwOutOctets = 0;

		//将所有端口的流量进行统计
		for (UINT i = 0; i < m_pTable->dwNumEntries; i++)
		{
			MIB_IFROW Row = m_pTable->table[i];
			if (Row.dwType <= 23)
			{
				dwInOctets += Row.dwInOctets;
				dwOutOctets += Row.dwOutOctets;
			}
		}

		dwBandIn = dwInOctets - dwLastIn;       //下载速度
		dwBandOut = dwOutOctets - dwLastOut;    //上传速速
		if (dwLastIn <= 0)
		{
			dwBandIn = 0;
		}
		else
		{
			dwBandIn = dwBandIn / 1024; //b转换成kb
		}

		if (dwLastOut <= 0)
		{
			dwBandOut = 0;
		}
		else
		{
			dwBandOut = dwBandOut / 1024;   //b转换成kb
		}

		dwLastIn = dwInOctets;
		dwLastOut = dwOutOctets;

		emit signal_intnetNumChanged(dwBandIn);
	}
	else
	{
		qDebug() << "error";
	}

	if (m_pTable != NULL) {
		delete[]m_pTable;
	}

	m_intnetTimer->start(INTNET_DETECTION_INTERVAL);
}


PerformanceWidget::PerformanceWidget(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	//setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Popup);
	qRegisterMetaType<DWORD>("DWORD");//注册DWORD类型
	m_overCpuThreadholdTimer = NULL;

	initWidget();
	initUIStyles();
	initText();
}

PerformanceWidget::~PerformanceWidget()
{
	m_pThread->quit();
}

QFont PerformanceWidget::getIconFont()
{
	QString strIconFilePath = "acs_client.ttf";
	static int nFontId = QFontDatabase::addApplicationFont(strIconFilePath);
	static QString strFontName = QFontDatabase::applicationFontFamilies(nFontId).at(0);
	static QFont iconFont(strFontName);
	iconFont.setPixelSize(12);
	iconFont.setStyleStrategy(QFont::PreferAntialias);
	return QFont(iconFont);
}


void PerformanceWidget::initWidget()
{
	//启动线程获取cpu、内存、网络
	m_pPerformanceThread = new PerformanceThread;
	m_overCpuThreadholdTimer = new QTimer;
	m_pThread = new QThread();
	m_pPerformanceThread->moveToThread(m_pThread);
	connect(m_pThread, SIGNAL(started()), m_pPerformanceThread, SLOT(start()));
	m_pThread->start();
	connect(m_pPerformanceThread, SIGNAL(signal_cpuPercentChanged(float)), this, SLOT(slot_cpuPercentChanged(float)));
	connect(m_pPerformanceThread, SIGNAL(signal_memoryPercentChanged(int)), this, SLOT(slot_memoryPercentChanged(int)));
	connect(m_pPerformanceThread, SIGNAL(signal_intnetNumChanged(DWORD)), this, SLOT(slot_intnetNumChanged(DWORD)));
	connect(m_overCpuThreadholdTimer, SIGNAL(timeout()), this, SLOT(slot_overCpuThreadholdTimeout()));
}

void PerformanceWidget::initUIStyles()
{
	ui.widget->setStyleSheet("QWidget#widget{background-color:#ffffff;}");
	ui.listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void PerformanceWidget::initText()
{
	//aci - wifi
	ui.intnetIcon->setFont(getIconFont());
	ui.memoryIcon->setFont(getIconFont());
	ui.cpuIcon->setFont(getIconFont());

	ui.intnetIcon->setText(aci_wifi);
	ui.cpuIcon->setText(aci_x062_cpu);	
	ui.memoryIcon->setText(aci_x063_memory);

	ui.cpuPercentLab->setText("--%");
	ui.memoryPercentLab->setText("--%");
	ui.intnetNumLab->setText("--KB/S");

	ui.intnetLab->setText(QStringLiteral("网络"));
	ui.memoryLab->setText(QStringLiteral("内存"));
}
/*
* status：状态，1绿2黄3红
*/
void PerformanceWidget::addItem(int status,QString value)
{
	QWidget* widget = new PerformaceItem(status,value);
	QListWidgetItem* item = new QListWidgetItem();
	ui.listWidget->insertItem(0,item);
	item->setSizeHint(QSize(widget->width(), widget->height()));
	ui.listWidget->setItemWidget(item, widget);
}

void PerformanceWidget::slot_cpuPercentChanged(float cpuPercent)
{
	ui.cpuPercentLab->setText(QString::number(cpuPercent, 'f', 2) + "%");
	if (cpuPercent < 0 || cpuPercent > 100)
	{
		ui.cpuPercentLab->setText("--%");
	}
	else
	{
		emit signal_overCpuThreshold();

		if (m_overCpuThreadholdTimer->isActive())
		{
			m_overCpuThreadholdTimer->stop();
		}
		m_overCpuThreadholdTimer->start(10000);
		QString cpuValue = QStringLiteral("CPU: ") + ui.cpuPercentLab->text();
		if (cpuPercent < 60)
			addItem(1, cpuValue);
		else if (cpuPercent < 80)
			addItem(2, cpuValue);
		else
			addItem(3, cpuValue);
	}
}

void PerformanceWidget::slot_memoryPercentChanged(int memoryPercent)
{
	ui.memoryPercentLab->setText(QString::number(memoryPercent) + "%");

	if (memoryPercent < 0 || memoryPercent > 100)
	{
		ui.memoryPercentLab->setText("--%");
	}
	QString memValue = QStringLiteral("内存: ") + ui.memoryPercentLab->text();

	if (memoryPercent < 60)
		addItem(1, memValue);
	else if (memoryPercent < 80)
		addItem(2, memValue);
	else
		addItem(3, memValue);
}

void PerformanceWidget::slot_intnetNumChanged(DWORD intnetNum)
{
	if (intnetNum < 0)
	{
		ui.intnetNumLab->setText("--KB/S");
	}
	else if (intnetNum < 1024)
	{
		ui.intnetNumLab->setText(QString::number(intnetNum) + "KB/S");
		addItem(1, QStringLiteral("网络: ") + ui.intnetNumLab->text());
	}
	else if (intnetNum >= 1024 && intnetNum < 1024 * 1024)
	{
		float num = (float)intnetNum / 1024;
		ui.intnetNumLab->setText(QString::number(num, 'f', 2) + "MB/S");
		addItem(2, QStringLiteral("网络: ") + ui.intnetNumLab->text());
	}
	else
	{
		float num = (float)intnetNum / (1024 * 1024);
		ui.intnetNumLab->setText(QString::number(num, 'f', 2) + "GB");
		addItem(3, QStringLiteral("网络: ") + ui.intnetNumLab->text());
	}
	
}

void PerformanceWidget::slot_overCpuThreadholdTimeout()
{
	if (m_overCpuThreadholdTimer->isActive())
	{
		m_overCpuThreadholdTimer->stop();
	}
}
