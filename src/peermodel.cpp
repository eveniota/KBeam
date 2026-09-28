// SPDX-FileCopyrightText: 2026 Mradul Pal <mradulpal@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "peermodel.h"

PeerModel::PeerModel(QObject* parent) : QAbstractListModel(parent)
{}

int PeerModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_peers.size();
}

QVariant PeerModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_peers.size()) {
        return {};
    }
    const PeerInfo &peer = m_peers.at(index.row());
    switch (role)
    {
        case NameRole: return peer.name;
        case MacRole: return peer.mac ;
        case UniRole: return peer.uni ;
        case HasWfdRole: return peer.hasWfd ;
        default: return {};
    }
}

QHash<int, QByteArray> PeerModel::roleNames() const
{
    return {
        {NameRole, "name"},
            {MacRole, "mac"},
            {UniRole, "uni"},
        {HasWfdRole, "hasWfd"},
    };
}

int PeerModel::role(const QByteArray &roleName) const
{
    return roleNames().key(roleName, -1);
}

void PeerModel::clear()
{
    beginResetModel();
    m_peers.clear();
    endResetModel();
}

void PeerModel::addPeer(const PeerInfo &info)
{
    const int row = m_peers.size();
    beginInsertRows(QModelIndex(), row, row);
    m_peers.append(info);
    endInsertRows();
}

void PeerModel::removePeer(const QString &uni)
{
    for (int i = 0; i < m_peers.size(); ++i)
    {
        if (m_peers[i].uni == uni)
        {
            beginRemoveRows(QModelIndex(), i, i);
            m_peers.removeAt(i);
            endRemoveRows();
            break;
        }
    }
}

QString PeerModel::peerUni(const QString &mac) const
{
    for (const auto &peer : m_peers) {
        if (peer.mac.compare(mac, Qt::CaseInsensitive) == 0) {
            return peer.uni;
        }
    }
    return {};
}

QString PeerModel::peerMac(const QString &uni) const
{
    for (const auto &peer : m_peers) {
        if (peer.uni == uni) {
            return peer.mac;
        }
    }
    return {};
}

QString PeerModel::peerName(const QString &identifier) const
{
    for (const auto &peer : m_peers) {
        if (peer.mac.compare(identifier, Qt::CaseInsensitive) == 0 || peer.uni == identifier) {
            return peer.name;
        }
    }
    return {};
}