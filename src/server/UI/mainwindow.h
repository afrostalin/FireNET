#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
public:
    void LogToOutput(ELogType type, const QString& msg);
    void SetServerStatus();
public slots:
    void CleanUp();
private slots:
    void on_Input_returnPressed();
private:
	void ClearOutput();
	void ClearStatus();
private:
    Ui::MainWindow *ui;
    int m_OutputItemID;
};

#endif // MAINWINDOW_H
