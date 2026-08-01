//
// Created by mradu1 on 7/25/26.
//

#pragma once

#include <QObject>
#include <QString>
#include <NetworkManagerQt/WifiP2PDevice>
#include "peermodel.h"

class P2PDiscovery : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(PeerModel* peers READ peers CONSTANT)
public:
    explicit P2PDiscovery(QObject *parent = nullptr);

    QString statusMessage() const;
    PeerModel* peers() const;

    Q_INVOKABLE void startDiscovery();
    Q_INVOKABLE void stopDiscovery();
    Q_INVOKABLE void connectToPeer(const QString &mac);
Q_SIGNALS:
    void statusMessageChanged();

private Q_SLOTS:
    void onPeerAppeared(const QString &uni);
    void onPeerDisappeared(const QString &uni);

private:
    void setStatusMessage(const QString &status);
    QString m_statusMessage;
    PeerModel* m_peers = nullptr;
    NetworkManager::WifiP2PDevice::Ptr m_device;
};
