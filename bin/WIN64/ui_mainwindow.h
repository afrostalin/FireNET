/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QPushButton *connectBtn;
    QSpinBox *port;
    QLineEdit *loginTxt;
    QLineEdit *passwordTxt;
    QLineEdit *nicknameTxt;
    QLineEdit *modelTxt;
    QPushButton *loginBtn;
    QPushButton *registerBtn;
    QPushButton *createProfileBtn;
    QPushButton *getProfileBtn;
    QPushButton *getShopBtn;
    QPushButton *buyItemBtn;
    QLineEdit *itemNameTxt;
    QLineEdit *friendNameTxt;
    QPushButton *addFriendBtn;
    QPushButton *removeFriendBtn;
    QPushButton *getStatsBtn;
    QPushButton *getAchievmentsBtn;
    QPushButton *getServerBtn;
    QLineEdit *mapNameTxt;
    QListWidget *logWindow;
    QPushButton *clearBtn;
    QPushButton *pushButton;
    QLineEdit *lineEdit;
    QPushButton *disconnectBtn;
    QPushButton *pushButton_2;
    QPushButton *stopStressTest;
    QLineEdit *chatTxt;
    QPushButton *sendGlobalChatBtn;
    QPushButton *sendPrivateBtn;
    QFrame *line;
    QLabel *label;
    QFrame *line_2;
    QFrame *line_3;
    QFrame *line_4;
    QLabel *label_2;
    QFrame *line_5;
    QFrame *line_6;
    QFrame *line_7;
    QFrame *line_8;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(741, 489);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        connectBtn = new QPushButton(centralWidget);
        connectBtn->setObjectName(QStringLiteral("connectBtn"));
        connectBtn->setGeometry(QRect(10, 40, 111, 23));
        port = new QSpinBox(centralWidget);
        port->setObjectName(QStringLiteral("port"));
        port->setGeometry(QRect(150, 10, 91, 22));
        port->setMaximum(99999);
        port->setValue(3322);
        loginTxt = new QLineEdit(centralWidget);
        loginTxt->setObjectName(QStringLiteral("loginTxt"));
        loginTxt->setGeometry(QRect(10, 70, 131, 23));
        passwordTxt = new QLineEdit(centralWidget);
        passwordTxt->setObjectName(QStringLiteral("passwordTxt"));
        passwordTxt->setGeometry(QRect(10, 100, 131, 23));
        nicknameTxt = new QLineEdit(centralWidget);
        nicknameTxt->setObjectName(QStringLiteral("nicknameTxt"));
        nicknameTxt->setGeometry(QRect(10, 130, 131, 23));
        modelTxt = new QLineEdit(centralWidget);
        modelTxt->setObjectName(QStringLiteral("modelTxt"));
        modelTxt->setGeometry(QRect(10, 160, 131, 23));
        loginBtn = new QPushButton(centralWidget);
        loginBtn->setObjectName(QStringLiteral("loginBtn"));
        loginBtn->setGeometry(QRect(150, 100, 91, 23));
        registerBtn = new QPushButton(centralWidget);
        registerBtn->setObjectName(QStringLiteral("registerBtn"));
        registerBtn->setGeometry(QRect(150, 70, 91, 23));
        createProfileBtn = new QPushButton(centralWidget);
        createProfileBtn->setObjectName(QStringLiteral("createProfileBtn"));
        createProfileBtn->setGeometry(QRect(150, 130, 91, 23));
        getProfileBtn = new QPushButton(centralWidget);
        getProfileBtn->setObjectName(QStringLiteral("getProfileBtn"));
        getProfileBtn->setGeometry(QRect(150, 160, 91, 23));
        getShopBtn = new QPushButton(centralWidget);
        getShopBtn->setObjectName(QStringLiteral("getShopBtn"));
        getShopBtn->setGeometry(QRect(10, 190, 231, 23));
        buyItemBtn = new QPushButton(centralWidget);
        buyItemBtn->setObjectName(QStringLiteral("buyItemBtn"));
        buyItemBtn->setGeometry(QRect(10, 250, 111, 23));
        itemNameTxt = new QLineEdit(centralWidget);
        itemNameTxt->setObjectName(QStringLiteral("itemNameTxt"));
        itemNameTxt->setGeometry(QRect(10, 220, 231, 23));
        friendNameTxt = new QLineEdit(centralWidget);
        friendNameTxt->setObjectName(QStringLiteral("friendNameTxt"));
        friendNameTxt->setGeometry(QRect(10, 280, 231, 23));
        addFriendBtn = new QPushButton(centralWidget);
        addFriendBtn->setObjectName(QStringLiteral("addFriendBtn"));
        addFriendBtn->setGeometry(QRect(10, 310, 111, 23));
        removeFriendBtn = new QPushButton(centralWidget);
        removeFriendBtn->setObjectName(QStringLiteral("removeFriendBtn"));
        removeFriendBtn->setGeometry(QRect(130, 310, 111, 23));
        getStatsBtn = new QPushButton(centralWidget);
        getStatsBtn->setObjectName(QStringLiteral("getStatsBtn"));
        getStatsBtn->setGeometry(QRect(10, 340, 231, 23));
        getAchievmentsBtn = new QPushButton(centralWidget);
        getAchievmentsBtn->setObjectName(QStringLiteral("getAchievmentsBtn"));
        getAchievmentsBtn->setGeometry(QRect(10, 370, 231, 23));
        getServerBtn = new QPushButton(centralWidget);
        getServerBtn->setObjectName(QStringLiteral("getServerBtn"));
        getServerBtn->setGeometry(QRect(10, 430, 231, 23));
        mapNameTxt = new QLineEdit(centralWidget);
        mapNameTxt->setObjectName(QStringLiteral("mapNameTxt"));
        mapNameTxt->setGeometry(QRect(10, 400, 231, 23));
        logWindow = new QListWidget(centralWidget);
        logWindow->setObjectName(QStringLiteral("logWindow"));
        logWindow->setGeometry(QRect(260, 110, 461, 331));
        clearBtn = new QPushButton(centralWidget);
        clearBtn->setObjectName(QStringLiteral("clearBtn"));
        clearBtn->setGeometry(QRect(260, 450, 461, 23));
        pushButton = new QPushButton(centralWidget);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(130, 250, 111, 23));
        lineEdit = new QLineEdit(centralWidget);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));
        lineEdit->setGeometry(QRect(10, 10, 131, 20));
        disconnectBtn = new QPushButton(centralWidget);
        disconnectBtn->setObjectName(QStringLiteral("disconnectBtn"));
        disconnectBtn->setGeometry(QRect(130, 40, 111, 23));
        pushButton_2 = new QPushButton(centralWidget);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));
        pushButton_2->setGeometry(QRect(10, 460, 111, 23));
        stopStressTest = new QPushButton(centralWidget);
        stopStressTest->setObjectName(QStringLiteral("stopStressTest"));
        stopStressTest->setGeometry(QRect(130, 460, 111, 23));
        chatTxt = new QLineEdit(centralWidget);
        chatTxt->setObjectName(QStringLiteral("chatTxt"));
        chatTxt->setGeometry(QRect(260, 30, 301, 20));
        sendGlobalChatBtn = new QPushButton(centralWidget);
        sendGlobalChatBtn->setObjectName(QStringLiteral("sendGlobalChatBtn"));
        sendGlobalChatBtn->setGeometry(QRect(570, 30, 71, 23));
        sendPrivateBtn = new QPushButton(centralWidget);
        sendPrivateBtn->setObjectName(QStringLiteral("sendPrivateBtn"));
        sendPrivateBtn->setGeometry(QRect(650, 30, 71, 23));
        line = new QFrame(centralWidget);
        line->setObjectName(QStringLiteral("line"));
        line->setGeometry(QRect(250, 10, 481, 20));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        label = new QLabel(centralWidget);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(450, 0, 71, 16));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        label->setFont(font);
        line_2 = new QFrame(centralWidget);
        line_2->setObjectName(QStringLiteral("line_2"));
        line_2->setGeometry(QRect(720, 20, 20, 41));
        line_2->setFrameShape(QFrame::VLine);
        line_2->setFrameShadow(QFrame::Sunken);
        line_3 = new QFrame(centralWidget);
        line_3->setObjectName(QStringLiteral("line_3"));
        line_3->setGeometry(QRect(240, 20, 20, 41));
        line_3->setFrameShape(QFrame::VLine);
        line_3->setFrameShadow(QFrame::Sunken);
        line_4 = new QFrame(centralWidget);
        line_4->setObjectName(QStringLiteral("line_4"));
        line_4->setGeometry(QRect(250, 50, 481, 20));
        line_4->setFrameShape(QFrame::HLine);
        line_4->setFrameShadow(QFrame::Sunken);
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(450, 80, 71, 16));
        label_2->setFont(font);
        line_5 = new QFrame(centralWidget);
        line_5->setObjectName(QStringLiteral("line_5"));
        line_5->setGeometry(QRect(250, 90, 481, 20));
        line_5->setFrameShape(QFrame::HLine);
        line_5->setFrameShadow(QFrame::Sunken);
        line_6 = new QFrame(centralWidget);
        line_6->setObjectName(QStringLiteral("line_6"));
        line_6->setGeometry(QRect(240, 100, 20, 381));
        line_6->setFrameShape(QFrame::VLine);
        line_6->setFrameShadow(QFrame::Sunken);
        line_7 = new QFrame(centralWidget);
        line_7->setObjectName(QStringLiteral("line_7"));
        line_7->setGeometry(QRect(720, 100, 20, 381));
        line_7->setFrameShape(QFrame::VLine);
        line_7->setFrameShadow(QFrame::Sunken);
        line_8 = new QFrame(centralWidget);
        line_8->setObjectName(QStringLiteral("line_8"));
        line_8->setGeometry(QRect(250, 470, 481, 20));
        line_8->setFrameShape(QFrame::HLine);
        line_8->setFrameShadow(QFrame::Sunken);
        MainWindow->setCentralWidget(centralWidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "FireNET Client", 0));
        connectBtn->setText(QApplication::translate("MainWindow", "Connect", 0));
        loginTxt->setText(QApplication::translate("MainWindow", "admin@noname.com", 0));
        passwordTxt->setText(QApplication::translate("MainWindow", "qwerty", 0));
        nicknameTxt->setText(QApplication::translate("MainWindow", "Administrator", 0));
        modelTxt->setText(QApplication::translate("MainWindow", "objects/characters/admin.cdf", 0));
        loginBtn->setText(QApplication::translate("MainWindow", "Login", 0));
        registerBtn->setText(QApplication::translate("MainWindow", "Register", 0));
        createProfileBtn->setText(QApplication::translate("MainWindow", "Create profile", 0));
        getProfileBtn->setText(QApplication::translate("MainWindow", "Get profle", 0));
        getShopBtn->setText(QApplication::translate("MainWindow", "Get shop items", 0));
        buyItemBtn->setText(QApplication::translate("MainWindow", "Buy item", 0));
        itemNameTxt->setText(QApplication::translate("MainWindow", "Pistol", 0));
        friendNameTxt->setText(QApplication::translate("MainWindow", "AfroStalin", 0));
        addFriendBtn->setText(QApplication::translate("MainWindow", "Add friend", 0));
        removeFriendBtn->setText(QApplication::translate("MainWindow", "Remove friend", 0));
        getStatsBtn->setText(QApplication::translate("MainWindow", "Get stats", 0));
        getAchievmentsBtn->setText(QApplication::translate("MainWindow", "Get achievments", 0));
        getServerBtn->setText(QApplication::translate("MainWindow", "Get game server", 0));
        mapNameTxt->setText(QApplication::translate("MainWindow", "map", 0));
        clearBtn->setText(QApplication::translate("MainWindow", "Clear log window", 0));
        pushButton->setText(QApplication::translate("MainWindow", "Remove item", 0));
        lineEdit->setText(QApplication::translate("MainWindow", "127.0.0.1", 0));
        disconnectBtn->setText(QApplication::translate("MainWindow", "Disconnect", 0));
        pushButton_2->setText(QApplication::translate("MainWindow", "Run stress test", 0));
        stopStressTest->setText(QApplication::translate("MainWindow", "Stop stress test", 0));
        sendGlobalChatBtn->setText(QApplication::translate("MainWindow", "Send global", 0));
        sendPrivateBtn->setText(QApplication::translate("MainWindow", "Send private", 0));
        label->setText(QApplication::translate("MainWindow", "Chat system", 0));
        label_2->setText(QApplication::translate("MainWindow", "Log window", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
