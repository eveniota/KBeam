//
// Created by mradu1 on 8/10/26.
//

#pragma once

#include <QObject>
#include <QString>

typedef struct _GstRTSPServer GstRTSPServer;

class WFDServer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
public:
    explicit WFDServer(QObject *parent = nullptr);

    QString statusMessage() const;
    Q_INVOKABLE void start(const QString &bindAddress);
    Q_INVOKABLE void stop();

Q_SIGNALS:
    void statusMessageChanged();

private:
    void setStatusMessage(const QString &message);
    QString m_statusMessage;
    GstRTSPServer *m_server = nullptr;
    unsigned int m_attachId = 0;
};
