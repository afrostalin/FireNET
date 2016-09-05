#pragma once
#include <QByteArray>
#include <QVariant>
#include <QVector>
#include <QString>
#include <QSharedPointer>
#include <hiredis/read.h>

#include "exception.h"

namespace RedisClient {
class Response
{    
    ADD_EXCEPTION

public:
    enum Type { Status, Error, Integer, Bulk, MultiBulk, Unknown };

public:
    Response();
    Response(const QByteArray &);
    virtual ~Response(void);

    QVariant getValue();        
    Type getType() const;
    QByteArray source();
    QString toRawString() const;

    bool isEmpty() const;
    bool isErrorMessage() const;
    bool isDisabledCommandErrorMessage() const;
    bool isOkMessage() const;
    bool isValid();
    bool isMessage() const;
    bool isArray() const;
    bool hasUnusedBuffer() const;

    // Pub/Sub support
    QByteArray getChannel() const;

    // Cluster support
    bool isAskRedirect() const;
    bool isMovedRedirect() const;
    QByteArray getRedirectionHost() const;
    uint getRedirectionPort() const;

public:
    void setSource(const QByteArray&);    
    void appendToSource(const QByteArray&);
    QByteArray getUnusedBuffer();
    void reset();

    static QString valueToHumanReadString(const QVariant&, int indentLevel=0);

protected:
    Type getResponseType(const QByteArray&) const;
    Type getResponseType(const char) const;

    bool parse();
    void feed(const QByteArray &buffer);

protected:
    QByteArray m_responseSource;
    QSharedPointer<redisReader> m_redisReader;
    QSharedPointer<QVariant> m_result;

private:
    /*
     * hiredis custom functions
     */
    static void *createStringObject(const redisReadTask *task, char *str, size_t len);
    static void *createArrayObject(const redisReadTask *t, int elements);
    static void *createIntegerObject(const redisReadTask *task, long long value);
    static void *createNilObject(const redisReadTask *task);
    static void freeObject(void *obj);

    static const redisReplyObjectFunctions defaultFunctions;

    static redisReader *redisReaderCreate(void)
    {
        return redisReaderCreateWithFunctions(const_cast<redisReplyObjectFunctions*>(&defaultFunctions));
    }
};
}

Q_DECLARE_METATYPE(QVector<QVariant*>)
