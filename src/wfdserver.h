//
// Created by mradu1 on 8/10/26.
//

#pragma once

#include <QObject>
#include <QString>
#include <QPointer>

class WfdClientSession;
typedef struct _GstRTSPClient GstRTSPClient;
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
    void playRequested(const QString &sinkIp, quint16 sinkPort);

private:
    void setStatusMessage(const QString &message);
    void handleClientConnected(GstRTSPClient *client);
    QPointer<WfdClientSession> m_session;
    QString m_statusMessage;
    QString m_bindAddress;
    GstRTSPServer *m_server = nullptr;
    unsigned int m_attachId = 0;
};
