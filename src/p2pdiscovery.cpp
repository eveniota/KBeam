//
// Created by mradu1 on 7/25/26.
//
#include "p2pdiscovery.h"
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Device>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDebug>

P2PDiscovery::P2PDiscovery(QObject* parent) : QObject(parent)
{
    for (const auto& device : NetworkManager::networkInterfaces())
    {
        if (device->type() == NetworkManager::Device::WifiP2P)
        {
            m_device = qSharedPointerObjectCast<NetworkManager::WifiP2PDevice>(device);
            if (m_device) {
                setStatusMessage(QStringLiteral("Ready on %1").arg(m_device->interfaceName()));
                connect(m_device.data(), &NetworkManager::WifiP2PDevice::peerAppeared, this, &P2PDiscovery::onPeerAppeared);
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
    const QString label = peer->name().isEmpty() ? peer->hardwareAddress() : peer->name();
    m_peerNames.append(label);
    Q_EMIT peersChanged();
}

QString P2PDiscovery::statusMessage() const
{
    return m_statusMessage;
}

QStringList P2PDiscovery::peerNames() const
{
    return m_peerNames;
}

void P2PDiscovery::startDiscovery()
{
    if (!m_device)
    {
        return;
    }
    m_peerNames.clear();
    Q_EMIT peersChanged();
    auto *watcher = new QDBusPendingCallWatcher(m_device->startFind(), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, [this, watcher](QDBusPendingCallWatcher *)
    {
        QDBusPendingReply<> reply = *watcher;
        if (reply.isError())
        {
            setStatusMessage(reply.error().message());
        } else
        {
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
    setStatusMessage(QStringLiteral("Discovery Stopped"));
}

void P2PDiscovery::setStatusMessage(const QString& status)
{
    if (status == m_statusMessage)
    {
        return;
    }
    m_statusMessage = status;
    Q_EMIT statusMessageChanged();
}

