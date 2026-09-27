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
    Q_PROPERTY(QString ipv4Address READ ipv4Address NOTIFY ipv4AddressChanged)
    Q_PROPERTY(PeerModel* peers READ peers CONSTANT)
public:
    enum State
    {
        Idle = 0,
        Discovering,
        Connecting,
        Connected,
        Error,
    };
    Q_ENUM(State)

    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    State state() const;
    explicit P2PDiscovery(QObject *parent = nullptr);

    QString statusMessage() const;
    PeerModel* peers() const;

    Q_INVOKABLE void startDiscovery();
    Q_INVOKABLE void stopDiscovery();
    Q_INVOKABLE void connectToPeer(const QString &mac);
    QString ipv4Address() const;
Q_SIGNALS:
    void statusMessageChanged();
    void stateChanged();
    void ipv4AddressChanged();
private Q_SLOTS:
    void onPeerAppeared(const QString &uni);
    void onPeerDisappeared(const QString &uni);

private:
    void setState(State state);
    void findDevice();
    bool ensureFirewallZone(QString *errorMessage) const;
    State m_state = Idle;
    void setStatusMessage(const QString &status);
    QString m_statusMessage;
    PeerModel* m_peers = nullptr;
    NetworkManager::WifiP2PDevice::Ptr m_device;
    void setIpv4Address(const QString &address);
    QString m_ipv4Address;
};
