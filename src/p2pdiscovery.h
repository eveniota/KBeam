//
// Created by mradu1 on 7/25/26.
//

#pragma once

#include <QObject>
#include <QString>
#include <NetworkManagerQt/WifiP2PDevice>
#include <QStringList>

class P2PDiscovery : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(QStringList peerNames READ peerNames NOTIFY peersChanged)
public:
    explicit P2PDiscovery(QObject *parent = nullptr);

    QString statusMessage() const;

    Q_INVOKABLE void startDiscovery();
    Q_INVOKABLE void stopDiscovery();

Q_SIGNALS:
    void statusMessageChanged();
    void peersChanged();

private Q_SLOTS:
    void onPeerAppeared(const QString &uni);

private:
    QString m_statusMessage;
    QStringList m_peerNames;
    NetworkManager::WifiP2PDevice::Ptr m_device;
};
