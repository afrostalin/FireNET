#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QTcpServer>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connected = false;
    stress_test = false;

    ui->disconnectBtn->setEnabled(false);
    ui->registerBtn->setEnabled(false);
    ui->loginBtn->setEnabled(false);
    ui->createProfileBtn->setEnabled(false);
    ui->getProfileBtn->setEnabled(false);
    ui->getShopBtn->setEnabled(false);
    ui->buyItemBtn->setEnabled(false);
    ui->pushButton->setEnabled(false);
    ui->addFriendBtn->setEnabled(false);
    ui->removeFriendBtn->setEnabled(false);
    ui->getStatsBtn->setEnabled(false);
    ui->getAchievmentsBtn->setEnabled(false);
    ui->getServerBtn->setEnabled(false);
    ui->pushButton_2->setEnabled(false);
    ui->stopStressTest->setEnabled(false);
    ui->sendGlobalChatBtn->setEnabled(false);
    ui->sendPrivateBtn->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onConnectedToServer()
{
    qDebug() << "Client connected";
    connected = true;
    ui->logWindow->addItem("Connection established");

    // Connect/Disconnect buttons
    ui->connectBtn->setEnabled(false);
    ui->disconnectBtn->setEnabled(true);
    //
    ui->registerBtn->setEnabled(true);
    ui->loginBtn->setEnabled(true);
    ui->createProfileBtn->setEnabled(true);
    ui->getProfileBtn->setEnabled(true);
    ui->getShopBtn->setEnabled(true);
    ui->buyItemBtn->setEnabled(true);
    ui->pushButton->setEnabled(true);
    ui->addFriendBtn->setEnabled(true);
    ui->removeFriendBtn->setEnabled(true);
    ui->getStatsBtn->setEnabled(true);
    ui->getAchievmentsBtn->setEnabled(true);
    ui->getServerBtn->setEnabled(true);
    ui->pushButton_2->setEnabled(true);
    ui->stopStressTest->setEnabled(true);
    ui->sendGlobalChatBtn->setEnabled(true);
    ui->sendPrivateBtn->setEnabled(true);
}

void MainWindow::onReadyRead()
{
    QByteArray data =  m_socket->readAll();

    //QByteArray data = m_socket->readAll();
    qDebug() << "Recived : " << data;
    ui->logWindow->addItem(data);
}

void MainWindow::onBytesWritten(qint64 bytes)
{
    qDebug() << "Bytes written";
}

void MainWindow::onDisconnected()
{
    qDebug() << "Server disconnected";
    connected = false;
    ui->logWindow->addItem("Connection lost!");

    // Connect/Disconnect buttons
    ui->connectBtn->setEnabled(true);
    ui->disconnectBtn->setEnabled(false);
    //
    ui->registerBtn->setEnabled(false);
    ui->loginBtn->setEnabled(false);
    ui->createProfileBtn->setEnabled(false);
    ui->getProfileBtn->setEnabled(false);
    ui->getShopBtn->setEnabled(false);
    ui->buyItemBtn->setEnabled(false);
    ui->pushButton->setEnabled(false);
    ui->addFriendBtn->setEnabled(false);
    ui->removeFriendBtn->setEnabled(false);
    ui->getStatsBtn->setEnabled(false);
    ui->getAchievmentsBtn->setEnabled(false);
    ui->getServerBtn->setEnabled(false);
    ui->pushButton_2->setEnabled(false);
    ui->stopStressTest->setEnabled(false);
    ui->sendGlobalChatBtn->setEnabled(false);
    ui->sendPrivateBtn->setEnabled(false);
}

void MainWindow::SendMessageToServer(QByteArray data)
{
    //QByteArray compressedData = qCompress(data,9);
    m_socket->write(data);
    m_socket->waitForBytesWritten(3);
}

void MainWindow::on_connectBtn_clicked()
{
    // Start connect to server
    int port = ui->port->value();

    m_socket = new QSslSocket(this);
    m_socket->addCaCertificates("key.pem");
    m_socket->connectToHostEncrypted("127.0.0.1",port);

    connect(m_socket, &QSslSocket::encrypted, this, &MainWindow::onConnectedToServer);
    connect(m_socket, &QSslSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(m_socket, &QSslSocket::disconnected, this, &MainWindow::onDisconnected);
    connect(m_socket, &QSslSocket::bytesWritten, this, &MainWindow::onBytesWritten);
}

void MainWindow::on_registerBtn_clicked()
{
    QString login = ui->loginTxt->text();
    QString password = ui->passwordTxt->text();
    QString query = "<query type = 'register'><data login='" + login + "' password = '" + password + "'/></query>";

    SendMessageToServer(query.toStdString().c_str());
}

void MainWindow::on_loginBtn_clicked()
{
    QString login = ui->loginTxt->text();
    QString password = ui->passwordTxt->text();
    QString query = "<query type = 'auth'><data login='" + login + "' password = '" + password + "'/></query>";

    SendMessageToServer(query.toStdString().c_str());
}

void MainWindow::on_createProfileBtn_clicked()
{
    QString nickname = ui->nicknameTxt->text();
    QString model = ui->modelTxt->text();
    QString query = "<query type = 'create_profile'><data nickname='" + nickname + "' model = '" + model + "'/></query>";

    SendMessageToServer(query.toStdString().c_str());
}

void MainWindow::on_getProfileBtn_clicked()
{
    QString query = "<query type = 'get_profile'/>";

    SendMessageToServer(query.toStdString().c_str());
}

void MainWindow::on_getShopBtn_clicked()
{
    QString query = "<query type = 'get_shop_items'/>";

    SendMessageToServer(query.toStdString().c_str());
}

void MainWindow::on_buyItemBtn_clicked()
{
    QString itemName = ui->itemNameTxt->text();
    QString query = "<query type = 'buy_item'><data item = '" + itemName + "'/></query>";

    SendMessageToServer(query.toStdString().c_str());
}

void MainWindow::on_addFriendBtn_clicked()
{
    QString friendName = ui->friendNameTxt->text();
    QString query = "<query type = 'add_friend'><data name = '" + friendName + "'/></query>";

    SendMessageToServer(query.toStdString().c_str());
}

void MainWindow::on_removeFriendBtn_clicked()
{
    QString friendName = ui->friendNameTxt->text();
    QString query = "<query type = 'remove_friend'><data name = '" + friendName + "'/></query>";

    SendMessageToServer(query.toStdString().c_str());
}

void MainWindow::on_getStatsBtn_clicked()
{
    QString query = "<query type = 'get_stats'/>";

    SendMessageToServer(query.toStdString().c_str());
}

void MainWindow::on_getAchievmentsBtn_clicked()
{
    QString query = "<query type = 'get_achievments'/>";

    SendMessageToServer(query.toStdString().c_str());
}

void MainWindow::on_getServerBtn_clicked()
{
    QString mapName = ui->mapNameTxt->text();
    QString query = "<query type = 'get_server'><data map = '" + mapName + "'/></query>";

    SendMessageToServer(query.toStdString().c_str());
}

void MainWindow::on_clearBtn_clicked()
{
    ui->logWindow->clear();
}

void MainWindow::on_pushButton_clicked()
{
    QString itemName = ui->itemNameTxt->text();
    QString query = "<query type = 'remove_item'><data name = '" + itemName + "'/></query>";

    SendMessageToServer(query.toStdString().c_str());
}

void MainWindow::on_disconnectBtn_clicked()
{
    m_socket->close();
    ui->connectBtn->setEnabled(true);
}

void MainWindow::on_pushButton_2_clicked()
{
    stress_test = true;

    QString query = "<query type = 'register'><data login='test' password = 'qwerty'/></query>";
    QString query1 = "<query type = 'auth'><data login='test' password = 'qwerty'/></query>";
    QString query2 = "<query type = 'create_profile'><data nickname='Test' model = 'test'/></query>";
    QString query3 = "<query type = 'get_profile'/>";
    QString query4 = "<query type = 'get_shop_items'/>";
    QString query5 = "<query type = 'buy_item'><data item = 'Grenade'/></query>";
    QString query6 = "<query type = 'remove_item'><data name = 'Grenade'/></query>";
    QString query7 = "<query type = 'add_friend'><data name = 'AfroStalin'/></query>";
    QString query8 = "<query type = 'remove_friend'><data name = 'AfroStalin'/></query>";
    QString query9 = "<query type = 'chat_message'><data message = 'Hello, world' to = 'all'/></query>";
    QString query10 = "<query type = 'chat_message'><data message = 'Hello, AfroStalin' to = 'AfroStalin'/></query>";

    qDebug() << "Stress test runned!";
    ui->logWindow->addItem("Stress test runned!");

    while(stress_test)
    {
        SendMessageToServer(query.toStdString().c_str());
        SendMessageToServer(query1.toStdString().c_str());
        SendMessageToServer(query2.toStdString().c_str());
        SendMessageToServer(query3.toStdString().c_str());
        SendMessageToServer(query4.toStdString().c_str());
        SendMessageToServer(query5.toStdString().c_str());
        SendMessageToServer(query6.toStdString().c_str());
        SendMessageToServer(query7.toStdString().c_str());
        SendMessageToServer(query8.toStdString().c_str());
        SendMessageToServer(query9.toStdString().c_str());
        SendMessageToServer(query10.toStdString().c_str());

        QEventLoop loop;
        QTimer::singleShot(33, &loop, SLOT(quit()));
        loop.exec();
    }
    qDebug() << "Stress test stopped!";
    ui->logWindow->addItem("Stress test stopped!");
}

void MainWindow::on_stopStressTest_clicked()
{
    stress_test = false;
}

void MainWindow::on_sendGlobalChatBtn_clicked()
{
    QString message = ui->chatTxt->text();
    if(message.isEmpty())
        return;


    QString query = "<query type = 'chat_message'><data message = '" + message + "' to = 'all'/></query>";
    SendMessageToServer(query.toStdString().c_str());

    ui->chatTxt->clear();
}

void MainWindow::on_sendPrivateBtn_clicked()
{
    QString message = ui->chatTxt->text();
    QString clientName = ui->friendNameTxt->text();
    if(message.isEmpty() || clientName.isEmpty())
        return;

    QString query = "<query type = 'chat_message'><data message = '" + message + "' to = '" + clientName + "'/></query>";
    SendMessageToServer(query.toStdString().c_str());

    ui->chatTxt->clear();
}
