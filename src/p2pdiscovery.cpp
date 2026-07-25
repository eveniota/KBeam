//
// Created by mradu1 on 7/25/26.
//
#include "p2pdiscovery.h"
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Device>

P2PDiscovery::P2PDiscovery(QObject* parent) : QObject(parent)
{
    for (const auto& device : NetworkManager::networkInterfaces())
    {
        if (device->type() == NetworkManager::Device::WifiP2P)
        {
            qSharedPointerObjectCast<NetworkManager::WifiP2PDevice>(device);
            if (m_device) {
                m_statusMessage = QStringLiteral("Ready on %1").arg(m_device->interfaceName());
                break;
            }
        }
    }
    if (!m_device)
    {
        m_statusMessage = QStringLiteral("No wifi-p2p device (need NetworkManager + wpa_supplicant P2P)");
    }
}

QString P2PDiscovery::statusMessage() const
{
    return m_statusMessage;
}

