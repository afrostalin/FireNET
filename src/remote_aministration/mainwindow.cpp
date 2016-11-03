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

    ui->disconnectBtn->setEnabled(false);
    ui->loginBtn->setEnabled(false);
    ui->sendCommandBtn->setEnabled(false);
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
    ui->loginBtn->setEnabled(true);
    ui->sendCommandBtn->setEnabled(true);
}

void MainWindow::onReadyRead()
{
    QByteArray data =  m_socket->readAll();

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
    ui->loginBtn->setEnabled(false);
    ui->sendCommandBtn->setEnabled(false);
}

void MainWindow::SendMessageToServer(QByteArray data)
{
    if(connected)
    {
        m_socket->write(data);
        m_socket->waitForBytesWritten(3);
    }
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

void MainWindow::on_loginBtn_clicked()
{
    QString login = ui->loginTxt->text();
    QString password = ui->passwordTxt->text();
    QString query = "<query type = 'admin_auth'><data login='" + login + "' password = '" + password + "'/></query>";

    SendMessageToServer(query.toStdString().c_str());
}

void MainWindow::on_clearBtn_clicked()
{
    ui->logWindow->clear();
}

void MainWindow::on_disconnectBtn_clicked()
{
    m_socket->close();
    ui->connectBtn->setEnabled(true);
}

void MainWindow::on_sendCommandBtn_clicked()
{
    QString data = ui->consoleTxt->text();
    QString query = "<query type = 'console_command'><data command='" + data + "'/></query>";

    SendMessageToServer(query.toStdString().c_str());

    ui->consoleTxt->clear();
}
