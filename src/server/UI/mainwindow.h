// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <QMainWindow>
#include <QTimer>
#include <QMutex>

namespace Ui
{
    class MainWindow;
}

enum ELogType
{
    ELog_Debug,
    ELog_Info,
    ELog_Warning,
    ELog_Error,
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public:
	void            LogToOutput(ELogType type, const QString& msg);
	void            ClearOutput();
private:
	void            ClearStatus() const;
public slots:
	void            CleanUp();
	void            UpdateServerStatus() const;
	void            EnableStressMode();
private slots:
	void            on_Input_returnPressed();
signals:
	void            stop();
	void            scroll();
private:
    Ui::MainWindow* ui;
    int             m_OutputItemID;
	QTimer          m_UpdateTimer;
	QMutex          m_Mutex;
};	