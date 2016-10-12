/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSql module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QSQL_IBASE_H
#define QSQL_IBASE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtSql/qsqldriver.h>
#include <ibase.h>

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_IBASE
#else
#define Q_EXPORT_SQLDRIVER_IBASE Q_SQL_EXPORT
#endif

QT_BEGIN_NAMESPACE

class QSqlResult;
class QIBaseDriverPrivate;

class Q_EXPORT_SQLDRIVER_IBASE QIBaseDriver : public QSqlDriver
{
    friend class QIBaseResultPrivate;
    Q_DECLARE_PRIVATE(QIBaseDriver)
    Q_OBJECT
public:
    explicit QIBaseDriver(QObject *parent = 0);
    explicit QIBaseDriver(isc_db_handle connection, QObject *parent = 0);
    virtual ~QIBaseDriver();
    bool hasFeature(DriverFeature f) const Q_DECL_OVERRIDE;
    bool open(const QString &db,
                   const QString &user,
                   const QString &password,
                   const QString &host,
                   int port,
                   const QString &connOpts) Q_DECL_OVERRIDE;
    bool open(const QString &db,
            const QString &user,
            const QString &password,
            const QString &host,
            int port) { return open(db, user, password, host, port, QString()); }
    void close() Q_DECL_OVERRIDE;
    QSqlResult *createResult() const Q_DECL_OVERRIDE;
    bool beginTransaction() Q_DECL_OVERRIDE;
    bool commitTransaction() Q_DECL_OVERRIDE;
    bool rollbackTransaction() Q_DECL_OVERRIDE;
    QStringList tables(QSql::TableType) const Q_DECL_OVERRIDE;

    QSqlRecord record(const QString& tablename) const Q_DECL_OVERRIDE;
    QSqlIndex primaryIndex(const QString &table) const Q_DECL_OVERRIDE;

    QString formatValue(const QSqlField &field, bool trimStrings) const Q_DECL_OVERRIDE;
    QVariant handle() const Q_DECL_OVERRIDE;

    QString escapeIdentifier(const QString &identifier, IdentifierType type) const Q_DECL_OVERRIDE;

    bool subscribeToNotification(const QString &name) Q_DECL_OVERRIDE;
    bool unsubscribeFromNotification(const QString &name) Q_DECL_OVERRIDE;
    QStringList subscribedToNotifications() const Q_DECL_OVERRIDE;

private Q_SLOTS:
    void qHandleEventNotification(void* updatedResultBuffer);
};

QT_END_NAMESPACE

#endif // QSQL_IBASE_H
