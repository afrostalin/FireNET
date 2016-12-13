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
    qDebug() << "Packet sended. Size " << bytes;
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
    ui->getServerBtn->setEnabled(false);
    ui->pushButton_2->setEnabled(false);
    ui->stopStressTest->setEnabled(false);
    ui->sendGlobalChatBtn->setEnabled(false);
    ui->sendPrivateBtn->setEnabled(false);
}

void MainWindow::SendMessageToServer(NetPacket &packet)
{
    m_socket->write(packet.toString());
    m_socket->waitForBytesWritten(3);
}

void MainWindow::on_connectBtn_clicked()
{
    // Start connect to server
    QString ip = ui->serverIP->text();
    int port = ui->port->value();

    m_socket = new QSslSocket(this);
    m_socket->addCaCertificates("key.pem");
    m_socket->connectToHostEncrypted(ip ,port);

    connect(m_socket, &QSslSocket::encrypted, this, &MainWindow::onConnectedToServer);
    connect(m_socket, &QSslSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(m_socket, &QSslSocket::disconnected, this, &MainWindow::onDisconnected);
    connect(m_socket, &QSslSocket::bytesWritten, this, &MainWindow::onBytesWritten);
}

void MainWindow::on_registerBtn_clicked()
{
    QString login = ui->loginTxt->text();
    QString password = ui->passwordTxt->text();

    NetPacket packet(net_Query);
    packet.WriteInt(net_query_register);
    packet.WriteString(login.toStdString());
    packet.WriteString(password.toStdString());

    SendMessageToServer(packet);
}

void MainWindow::on_loginBtn_clicked()
{
    QString login = ui->loginTxt->text();
    QString password = ui->passwordTxt->text();

    NetPacket packet(net_Query);
    packet.WriteInt(net_query_auth);
    packet.WriteString(login.toStdString());
    packet.WriteString(password.toStdString());

    SendMessageToServer(packet);
}

void MainWindow::on_createProfileBtn_clicked()
{
    QString nickname = ui->nicknameTxt->text();
    QString model = ui->modelTxt->text();

    NetPacket packet(net_Query);
    packet.WriteInt(net_query_create_profile);
    packet.WriteString(nickname.toStdString());
    packet.WriteString(model.toStdString());

    SendMessageToServer(packet);
}

void MainWindow::on_getProfileBtn_clicked()
{
    NetPacket packet(net_Query);
    packet.WriteInt(net_query_get_profile);
    SendMessageToServer(packet);
}

void MainWindow::on_getShopBtn_clicked()
{
    NetPacket packet(net_Query);
    packet.WriteInt(net_query_get_shop);
    SendMessageToServer(packet);
}

void MainWindow::on_buyItemBtn_clicked()
{
    QString itemName = ui->itemNameTxt->text();

    NetPacket packet(net_Query);
    packet.WriteInt(net_query_buy_item);
    packet.WriteString(itemName.toStdString());

    SendMessageToServer(packet);
}

void MainWindow::on_addFriendBtn_clicked()
{
    QString friendName = ui->friendNameTxt->text();

    NetPacket packet(net_Query);
    packet.WriteInt(net_query_add_friend);
    packet.WriteString(friendName.toStdString());

    SendMessageToServer(packet);
}

void MainWindow::on_removeFriendBtn_clicked()
{
    QString friendName = ui->friendNameTxt->text();

    NetPacket packet(net_Query);
    packet.WriteInt(net_query_remove_friend);
    packet.WriteString(friendName.toStdString());

    SendMessageToServer(packet);
}

void MainWindow::on_getServerBtn_clicked()
{
    QString mapName = ui->mapNameTxt->text();
    QString gameRules = ui->gamerulesTxt->text();
    QString serverName = ui->serverNameTxt->text();

    NetPacket packet(net_Query);
    packet.WriteInt(net_query_get_server);
    packet.WriteString(mapName.toStdString());
    packet.WriteString(gameRules.toStdString());
    packet.WriteString(serverName.toStdString());

    SendMessageToServer(packet);
}

void MainWindow::on_clearBtn_clicked()
{
    ui->logWindow->clear();
}

// Remove item
void MainWindow::on_pushButton_clicked()
{
    QString itemName = ui->itemNameTxt->text();

    NetPacket packet(net_Query);
    packet.WriteInt(net_query_remove_item);
    packet.WriteString(itemName.toStdString());

    SendMessageToServer(packet);
}

void MainWindow::on_disconnectBtn_clicked()
{
    m_socket->close();
    ui->connectBtn->setEnabled(true);
}

void MainWindow::on_pushButton_2_clicked()
{
    stress_test = true;

    qDebug() << "Stress test runned!";
    ui->logWindow->addItem("Stress test runned!");

    NetPacket login_packet(net_Query);
    login_packet.WriteInt(net_query_auth);
    login_packet.WriteString("test@test");
    login_packet.WriteString("testtest");

    NetPacket reg_packet(net_Query);
    reg_packet.WriteInt(net_query_register);
    login_packet.WriteString("test@test");
    login_packet.WriteString("testtest");

    NetPacket create_profile(net_Query);
    create_profile.WriteInt(net_query_create_profile);
    create_profile.WriteString("Test");
    create_profile.WriteString("objects/character/human/sdk_player/sdk_player.cdf");

    NetPacket get_profile(net_Query);
    get_profile.WriteInt(net_query_get_profile);

    NetPacket get_shop(net_Query);
    get_shop.WriteInt(net_query_get_shop);

    NetPacket buy_item(net_Query);
    buy_item.WriteInt(net_query_buy_item);
    buy_item.WriteString("pistol");

    NetPacket add_friend(net_Query);
    add_friend.WriteInt(net_query_add_friend);
    add_friend.WriteString("AfroStalin");

    NetPacket remove_friend(net_Query);
    remove_friend.WriteInt(net_query_remove_friend);
    remove_friend.WriteString("AfroStalin");

    NetPacket get_server(net_Query);
    get_server.WriteInt(net_query_get_server);
    get_server.WriteString("Airfield");
    get_server.WriteString("");
    get_server.WriteString("");

    NetPacket remove_item(net_Query);
    remove_item.WriteInt(net_query_remove_item);
    remove_item.WriteString("pistol");

    while(stress_test)
    {
        SendMessageToServer(login_packet);
        SendMessageToServer(reg_packet);
        SendMessageToServer(create_profile);
        SendMessageToServer(get_profile);
        SendMessageToServer(get_shop);
        SendMessageToServer(buy_item);
        SendMessageToServer(add_friend);
        SendMessageToServer(remove_friend);
        SendMessageToServer(get_server);
        SendMessageToServer(remove_item);

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

    NetPacket packet(net_Query);
    packet.WriteInt(net_query_send_chat_msg);
    packet.WriteString(message.toStdString());
    packet.WriteString("all");

    SendMessageToServer(packet);

    ui->chatTxt->clear();
}

void MainWindow::on_sendPrivateBtn_clicked()
{
    QString message = ui->chatTxt->text();
    QString clientName = ui->friendNameTxt->text();
    if(message.isEmpty() || clientName.isEmpty())
        return;

    NetPacket packet(net_Query);
    packet.WriteInt(net_query_send_chat_msg);
    packet.WriteString(message.toStdString());
    packet.WriteString(clientName.toStdString());

    SendMessageToServer(packet);

    ui->chatTxt->clear();
}
