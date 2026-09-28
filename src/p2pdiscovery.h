// SPDX-FileCopyrightText: 2026 Mradul Pal <mradulpal@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QString>
#include <NetworkManagerQt/WifiP2PDevice>
#include <NetworkManagerQt/ActiveConnection>
#include "peermodel.h"

class P2PDiscovery : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(QString ipv4Address READ ipv4Address NOTIFY ipv4AddressChanged)
    Q_PROPERTY(QString activePeerMac READ activePeerMac NOTIFY activePeerChanged)
    Q_PROPERTY(QString activePeerName READ activePeerName NOTIFY activePeerChanged)
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
    QString activePeerMac() const;
    QString activePeerName() const;
    PeerModel* peers() const;

    Q_INVOKABLE void startDiscovery();
    Q_INVOKABLE void stopDiscovery();
    Q_INVOKABLE void connectToPeer(const QString &mac);
    Q_INVOKABLE void disconnectPeer();
    QString ipv4Address() const;
Q_SIGNALS:
    void statusMessageChanged();
    void stateChanged();
    void ipv4AddressChanged();
    void activePeerChanged();
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
    QString m_activePeerMac;
    QString m_activePeerName;
    PeerModel* m_peers = nullptr;
    NetworkManager::WifiP2PDevice::Ptr m_device;
    NetworkManager::ActiveConnection::Ptr m_activeConnection;
    void setIpv4Address(const QString &address);
    QString m_ipv4Address;
};
