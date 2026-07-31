//
// Created by mradu1 on 7/31/26.
//

#include "peermodel.h"

PeerModel::PeerModel(QObject* parent) : QAbstractListModel(parent)
{

}

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
        default: return {};
    }
}

QHash<int, QByteArray> PeerModel::roleNames() const
{
    return {
        {NameRole, "name"},
            {MacRole, "mac"},
            {UniRole, "uni"}
    };
}

void PeerModel::clear()
{
    beginResetModel();
    m_peers.clear();
    endResetModel();
}

void PeerModel::addPeer(const PeerInfo &peer)
{
    const int row = m_peers.size();
    beginInsertRows(QModelIndex(), row, row);
    m_peers.append(peer);
    endInsertRows();
}