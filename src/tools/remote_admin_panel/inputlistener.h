// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <QObject>

class InputListener : public QObject
{
    Q_OBJECT
public:
    explicit InputListener(QObject *parent = 0);
public slots:
    void Run() const;
};
