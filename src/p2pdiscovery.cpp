//
// Created by mradu1 on 7/25/26.
//
#include "p2pdiscovery.h"
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/WifiP2PSetting>
#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/IpConfig>
#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/Device>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDebug>
#include <QUuid>
#include "peermodel.h"

P2PDiscovery::P2PDiscovery(QObject* parent) : QObject(parent)
{
    m_peers = new PeerModel(this);
    for (const auto& device : NetworkManager::networkInterfaces())
    {
        if (device->type() == NetworkManager::Device::WifiP2P)
        {
            m_device = qSharedPointerObjectCast<NetworkManager::WifiP2PDevice>(device);
            if (m_device) {
                setStatusMessage(QStringLiteral("Ready on %1").arg(m_device->interfaceName()));
                connect(m_device.data(), &NetworkManager::WifiP2PDevice::peerAppeared, this, &P2PDiscovery::onPeerAppeared);
                connect(m_device.data(), &NetworkManager::WifiP2PDevice::peerDisappeared, this, &P2PDiscovery::onPeerDisappeared);
                break;
            }
        }
    }
    if (!m_device)
    {
        setStatusMessage(QStringLiteral("No wifi-p2p device..."));
    }

}

void P2PDiscovery::onPeerAppeared(const QString& uni)
{
    const auto peer = m_device->findPeer(uni);
    if (!peer)
    {
        return;
    }
    PeerInfo peerInfo;
    peerInfo.name = peer->name().isEmpty() ? peer->hardwareAddress(): peer->name();
    peerInfo.mac = peer->hardwareAddress();
    peerInfo.uni = uni;
    peerInfo.hasWfd = !peer->wfdIEs().isEmpty();
    m_peers->addPeer(peerInfo);
}

P2PDiscovery::State P2PDiscovery::state() const
{
    return m_state;
}

PeerModel* P2PDiscovery::peers() const
{
    return m_peers;
}

QString P2PDiscovery::statusMessage() const
{
    return m_statusMessage;
}

void P2PDiscovery::setStatusMessage(const QString &status)
{
    if (status == m_statusMessage) {
        return;
    }
    m_statusMessage = status;
    Q_EMIT statusMessageChanged();
}

void P2PDiscovery::startDiscovery()
{
    if (!m_device)
    {
        return;
    }
    m_peers->clear();
    auto *watcher = new QDBusPendingCallWatcher(m_device->startFind(), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, [this, watcher](QDBusPendingCallWatcher *)
    {
        QDBusPendingReply<> reply = *watcher;
        if (reply.isError())
        {
            setState(Error);
            setStatusMessage(QStringLiteral("Discovery Failed"));
        } else
        {
            setState(Discovering);
            setStatusMessage(QStringLiteral("Discovering..."));
        }
        watcher->deleteLater();
    });
}

void P2PDiscovery::stopDiscovery()
{
    if (!m_device)
    {
        return;
    }
    m_device->stopFind();
    setState(Idle);
    setStatusMessage(QStringLiteral("Discovery Stopped"));
}

void P2PDiscovery::connectToPeer(const QString& mac)
{
    if (!m_device)
    {
        return;
    }
    NetworkManager::ConnectionSettings settings(NetworkManager::ConnectionSettings::WifiP2P);
    settings.setId(QStringLiteral("KCast-") + mac);
    settings.setUuid(QUuid::createUuid().toString(QUuid::WithoutBraces));

    auto wifiP2P = settings.setting(NetworkManager::Setting::WifiP2P).staticCast<NetworkManager::WifiP2PSetting>();
    if (!wifiP2P)
    {
        setState(Error);
        setStatusMessage(QStringLiteral("Missing wifi-p2p setting"));
        return;
    }
    wifiP2P->setPeer(mac);
    static const QByteArray wfdSourceIEs =
        QByteArray::fromRawData("\x00\x00\x06\x00\x90\x1c\x44\x00\xc8", 9);
    wifiP2P->setWfdIEs(wfdSourceIEs);
    wifiP2P->setPeer(mac);
    wifiP2P->setInitialized(true);
    setState(Connecting);
    setStatusMessage(QStringLiteral("Connecting to ") + mac);
    auto *watcher = new QDBusPendingCallWatcher(
        NetworkManager::addAndActivateConnection2(
            settings.toMap(),
            m_device->uni(),
            QString(),
            {{QStringLiteral("persist") , QStringLiteral("volatile")}}
        ),
        this);

        connect(watcher, &QDBusPendingCallWatcher::finished, this , [this, watcher](QDBusPendingCallWatcher *)
        {
            QDBusPendingReply<QDBusObjectPath, QDBusObjectPath> reply = *watcher;
            if (reply.isError())
            {
                setState(Error);
                setStatusMessage(reply.error().message());
                watcher->deleteLater();
                return;
            }
            const QDBusObjectPath activePath = reply.argumentAt<1>();
            NetworkManager::ActiveConnection::Ptr active = NetworkManager::findActiveConnection(activePath.path());

            if (!active)
            {
                setState(Error);
                setStatusMessage(QStringLiteral("No active connection found"));
                watcher->deleteLater();
                return;
            }
            auto reportIp = [this, active]() {
                if (!active || active->state() != NetworkManager::ActiveConnection::Activated) {
                   return;
                }
                const NetworkManager::IpConfig cfg = active->ipV4Config();
                if (!cfg.isValid() || cfg.addresses().isEmpty()) {
                   return;
                }
                const QString ip = cfg.addresses().constFirst().ip().toString();
                setIpv4Address(ip);
                setState(Connected);
                setStatusMessage(QStringLiteral("Connected — ") + ip);
            };

            connect(active.data(), &NetworkManager::ActiveConnection::stateChanged, this, reportIp);
            connect(active.data(), &NetworkManager::ActiveConnection::ipV4ConfigChanged, this, reportIp);
            reportIp();

            watcher->deleteLater();
        });
}

void P2PDiscovery::setState(State state)
{
    if (m_state == state)
    {
        return;
    }
    m_state = state;
    Q_EMIT stateChanged();
}


void P2PDiscovery::onPeerDisappeared(const QString &uni)
{
    m_peers->removePeer(uni);
}

QString P2PDiscovery::ipv4Address() const
{
    return m_ipv4Address;
}

void P2PDiscovery::setIpv4Address(const QString &address )
{
    if (address == m_ipv4Address)
    {
        return;
    }
    m_ipv4Address = address;
    Q_EMIT ipv4AddressChanged();
}



