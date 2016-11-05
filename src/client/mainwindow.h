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
    void on_registerBtn_clicked();

    void on_loginBtn_clicked();

    void on_createProfileBtn_clicked();

    void on_getProfileBtn_clicked();

    void on_getShopBtn_clicked();

    void on_buyItemBtn_clicked();

    void on_addFriendBtn_clicked();

    void on_removeFriendBtn_clicked();

    void on_getServerBtn_clicked();

    void on_clearBtn_clicked();

    void on_pushButton_clicked();

    void on_disconnectBtn_clicked();

    void on_pushButton_2_clicked();

    void on_stopStressTest_clicked();

    void on_sendGlobalChatBtn_clicked();

    void on_sendPrivateBtn_clicked();

public slots:
    void onConnectedToServer();
    void onReadyRead();
    void onBytesWritten(qint64 bytes);
    void onDisconnected();

private:
    Ui::MainWindow *ui;
    bool connected;
    bool stress_test;
    QSslSocket* m_socket;
private:
    void SendMessageToServer(QByteArray data);
};

#endif // MAINWINDOW_H
