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
    QPushButton *loginBtn;
    QListWidget *logWindow;
    QPushButton *clearBtn;
    QLineEdit *lineEdit;
    QPushButton *disconnectBtn;
    QLineEdit *consoleTxt;
    QPushButton *sendCommandBtn;
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
        MainWindow->resize(420, 553);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        connectBtn = new QPushButton(centralWidget);
        connectBtn->setObjectName(QStringLiteral("connectBtn"));
        connectBtn->setGeometry(QRect(250, 10, 81, 23));
        port = new QSpinBox(centralWidget);
        port->setObjectName(QStringLiteral("port"));
        port->setGeometry(QRect(150, 10, 91, 22));
        port->setMaximum(99999);
        port->setValue(64000);
        loginTxt = new QLineEdit(centralWidget);
        loginTxt->setObjectName(QStringLiteral("loginTxt"));
        loginTxt->setGeometry(QRect(10, 40, 131, 23));
        passwordTxt = new QLineEdit(centralWidget);
        passwordTxt->setObjectName(QStringLiteral("passwordTxt"));
        passwordTxt->setGeometry(QRect(150, 40, 131, 23));
        loginBtn = new QPushButton(centralWidget);
        loginBtn->setObjectName(QStringLiteral("loginBtn"));
        loginBtn->setGeometry(QRect(290, 40, 121, 23));
        logWindow = new QListWidget(centralWidget);
        logWindow->setObjectName(QStringLiteral("logWindow"));
        logWindow->setGeometry(QRect(20, 100, 381, 331));
        clearBtn = new QPushButton(centralWidget);
        clearBtn->setObjectName(QStringLiteral("clearBtn"));
        clearBtn->setGeometry(QRect(20, 440, 381, 23));
        lineEdit = new QLineEdit(centralWidget);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));
        lineEdit->setGeometry(QRect(10, 10, 131, 20));
        disconnectBtn = new QPushButton(centralWidget);
        disconnectBtn->setObjectName(QStringLiteral("disconnectBtn"));
        disconnectBtn->setGeometry(QRect(340, 10, 71, 23));
        consoleTxt = new QLineEdit(centralWidget);
        consoleTxt->setObjectName(QStringLiteral("consoleTxt"));
        consoleTxt->setGeometry(QRect(20, 510, 301, 20));
        sendCommandBtn = new QPushButton(centralWidget);
        sendCommandBtn->setObjectName(QStringLiteral("sendCommandBtn"));
        sendCommandBtn->setGeometry(QRect(330, 510, 71, 23));
        line = new QFrame(centralWidget);
        line->setObjectName(QStringLiteral("line"));
        line->setGeometry(QRect(10, 490, 401, 20));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        label = new QLabel(centralWidget);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(130, 480, 141, 16));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        label->setFont(font);
        line_2 = new QFrame(centralWidget);
        line_2->setObjectName(QStringLiteral("line_2"));
        line_2->setGeometry(QRect(400, 500, 20, 41));
        line_2->setFrameShape(QFrame::VLine);
        line_2->setFrameShadow(QFrame::Sunken);
        line_3 = new QFrame(centralWidget);
        line_3->setObjectName(QStringLiteral("line_3"));
        line_3->setGeometry(QRect(0, 500, 20, 41));
        line_3->setFrameShape(QFrame::VLine);
        line_3->setFrameShadow(QFrame::Sunken);
        line_4 = new QFrame(centralWidget);
        line_4->setObjectName(QStringLiteral("line_4"));
        line_4->setGeometry(QRect(10, 530, 401, 20));
        line_4->setFrameShape(QFrame::HLine);
        line_4->setFrameShadow(QFrame::Sunken);
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(170, 70, 71, 16));
        label_2->setFont(font);
        line_5 = new QFrame(centralWidget);
        line_5->setObjectName(QStringLiteral("line_5"));
        line_5->setGeometry(QRect(10, 80, 401, 20));
        line_5->setFrameShape(QFrame::HLine);
        line_5->setFrameShadow(QFrame::Sunken);
        line_6 = new QFrame(centralWidget);
        line_6->setObjectName(QStringLiteral("line_6"));
        line_6->setGeometry(QRect(0, 90, 20, 381));
        line_6->setFrameShape(QFrame::VLine);
        line_6->setFrameShadow(QFrame::Sunken);
        line_7 = new QFrame(centralWidget);
        line_7->setObjectName(QStringLiteral("line_7"));
        line_7->setGeometry(QRect(400, 90, 20, 381));
        line_7->setFrameShape(QFrame::VLine);
        line_7->setFrameShadow(QFrame::Sunken);
        line_8 = new QFrame(centralWidget);
        line_8->setObjectName(QStringLiteral("line_8"));
        line_8->setGeometry(QRect(10, 460, 401, 20));
        line_8->setFrameShape(QFrame::HLine);
        line_8->setFrameShadow(QFrame::Sunken);
        MainWindow->setCentralWidget(centralWidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "FireNET administration panel", 0));
        connectBtn->setText(QApplication::translate("MainWindow", "Connect", 0));
        loginTxt->setText(QApplication::translate("MainWindow", "admin", 0));
        passwordTxt->setText(QApplication::translate("MainWindow", "qwerty", 0));
        loginBtn->setText(QApplication::translate("MainWindow", "Login", 0));
        clearBtn->setText(QApplication::translate("MainWindow", "Clear log window", 0));
        lineEdit->setText(QApplication::translate("MainWindow", "127.0.0.1", 0));
        disconnectBtn->setText(QApplication::translate("MainWindow", "Disconnect", 0));
        sendCommandBtn->setText(QApplication::translate("MainWindow", "Send", 0));
        label->setText(QApplication::translate("MainWindow", "Send console command", 0));
        label_2->setText(QApplication::translate("MainWindow", "Log window", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
