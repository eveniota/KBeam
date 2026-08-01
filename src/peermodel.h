//
// Created by mradu1 on 7/31/26.
//

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
};

class PeerModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        MacRole,
        UniRole,
    };
    Q_ENUM(Roles)

    explicit PeerModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    void clear();
    void addPeer(const PeerInfo &info);
    void removePeer(const QString &uni);

private:
    QList<PeerInfo> m_peers;
};

