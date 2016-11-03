#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSslSocket>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_connectBtn_clicked();
    void on_loginBtn_clicked();
    void on_clearBtn_clicked();
    void on_disconnectBtn_clicked();

    void on_sendCommandBtn_clicked();

public slots:
    void onConnectedToServer();
    void onReadyRead();
    void onBytesWritten(qint64 bytes);
    void onDisconnected();

private:
    Ui::MainWindow *ui;
    bool connected;
    QSslSocket* m_socket;
private:
    void SendMessageToServer(QByteArray data);
};

#endif // MAINWINDOW_H
