// SPDX-FileCopyrightText: 2026 Mradul Pal <mradulpal@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QHash>
#include <QVariant>
#include <QByteArray>
#include <QList>

struct PeerInfo
{
    QString name;
    QString mac;
    QString uni;
    bool hasWfd = false;
};

class PeerModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        MacRole,
        UniRole,
        HasWfdRole,
    };
    Q_ENUM(Roles)
    explicit PeerModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    Q_INVOKABLE int role(const QByteArray &roleName) const;
    void clear();
    void addPeer(const PeerInfo &info);
    void removePeer(const QString &uni);
    QString peerUni(const QString &mac) const;
    QString peerMac(const QString &uni) const;
    QString peerName(const QString &identifier) const;

private:
    QList<PeerInfo> m_peers;
};

