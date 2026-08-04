//
// Created by mradu1 on 8/2/26.
//

#pragma once

#include <QObject>
#include <QString>
#include <QDBusObjectPath>

class ScreencastPortal : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
public:
    explicit ScreencastPortal(QObject *parent = nullptr);
    QString statusMessage() const;
    Q_INVOKABLE void start();

Q_SIGNALS:
    void statusMessageChanged();
    void started(int fd, uint nodeId);
    void failed(const QString &message);

private Q_SLOTS:
    void onCreateSessionResponse(uint response, const QVariantMap &results);
    void onSelectSourcesResponse(uint response, const QVariantMap &results);
    void onStartResponse(uint response, const QVariantMap &results);
private:
    void setStatusMessage(const QString &status);
    QString makeToken(const QString &prefix) const;
    void createSession();
    void selectSources();
    void startSession();
    void openPipeWireRemote(uint nodeId);
    QString m_statusMessage;
    QDBusObjectPath m_sessionPath;
};
