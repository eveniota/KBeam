//
// Created by mradu1 on 7/25/26.
//
#include "p2pdiscovery.h"
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Device>
#include <QDebug>

P2PDiscovery::P2PDiscovery(QObject* parent) : QObject(parent)
{
    for (const auto& device : NetworkManager::networkInterfaces())
    {
        if (device->type() == NetworkManager::Device::WifiP2P)
        {
            m_device = qSharedPointerObjectCast<NetworkManager::WifiP2PDevice>(device);
            if (m_device) {
                m_statusMessage = QStringLiteral("Ready on %1").arg(m_device->interfaceName());
                connect(m_device.data(), &NetworkManager::WifiP2PDevice::peerAppeared, this, &P2PDiscovery::onPeerAppeared);
                break;
            }
        }
    }
    if (!m_device)
    {
        m_statusMessage = QStringLiteral("No wifi-p2p device (need NetworkManager + wpa_supplicant P2P)");
    }

}

void P2PDiscovery::onPeerAppeared(const QString& uni)
{
    const auto peer = m_device->findPeer(uni);
    if (!peer)
    {
        return;
    }
    qInfo() << "Peer found: "
            << peer->name()
            << peer->hardwareAddress()
            << "WFD bytes: " << peer->wfdIEs().size();
}

QString P2PDiscovery::statusMessage() const
{
    return m_statusMessage;
}

