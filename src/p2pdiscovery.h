//
// Created by mradu1 on 7/25/26.
//

#pragma once

#include <QObject>
#include <QString>
#include <NetworkManagerQt/WifiP2PDevice>

class P2PDiscovery : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
public:
    explicit P2PDiscovery(QObject *parent = nullptr);

    QString statusMessage() const;

Q_SIGNALS:
    void statusMessageChanged();

private:
    QString m_statusMessage;
    NetworkManager::WifiP2PDevice::Ptr m_device;

};
