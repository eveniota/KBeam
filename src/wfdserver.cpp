// SPDX-FileCopyrightText: 2026 Mradul Pal <mradulpal@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wfdserver.h"
#include <QDebug>
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

#include "wfdclientsession.h"

namespace
{
    constexpr char rtspService[] = "7236";
    constexpr char rtspMount[] = "/wfd1.0";
}

WFDServer::WFDServer(QObject *parent) : QObject(parent), m_server(nullptr), m_attachId(0)
{
    gst_init(nullptr, nullptr);
}

QString WFDServer::statusMessage() const
{
    return m_statusMessage;
}

void WFDServer::setServerAddress(const QString &address)
{
    if (!address.isEmpty()) {
        m_bindAddress = address;
        qDebug() << "KBeam: Updated WFD server RTSP address to:" << m_bindAddress;
    }
}

void  WFDServer::start(const QString &bindAddress)
{
    if (m_server && m_attachId != 0) {
        if (!bindAddress.isEmpty() && bindAddress != QStringLiteral("0.0.0.0")) {
            setServerAddress(bindAddress);
        }
        return;
    }

    stop();
    m_server = gst_rtsp_server_new();
    m_bindAddress = (!bindAddress.isEmpty() && bindAddress != QStringLiteral("0.0.0.0")) ? bindAddress : QStringLiteral("127.0.0.1");

    g_signal_connect(
        m_server, "client-connected",
        G_CALLBACK(+[](GstRTSPServer *, GstRTSPClient *client, gpointer userData) {
            static_cast<WFDServer *>(userData)->handleClientConnected(client);
        }), this);
    gst_rtsp_server_set_service(m_server, rtspService);
    gst_rtsp_server_set_address(m_server, "0.0.0.0");

    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(m_server);
    GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();

    gst_rtsp_media_factory_set_launch(
                factory,
                "( intervideosrc channel=kbeam-desktop "
                "! videoconvert "
                "! video/x-raw,format=I420 "
                "! x264enc tune=zerolatency speed-preset=ultrafast key-int-max=30 "
                "! video/x-h264,profile=baseline,stream-format=byte-stream "
                "! mpegtsmux alignment=7 "
                "! rtpmp2tpay name=pay0 pt=33 )");

    gst_rtsp_media_factory_set_shared(factory, TRUE);

    gst_rtsp_mount_points_add_factory(mounts, rtspMount, factory);
    g_object_unref(mounts);

    m_attachId = gst_rtsp_server_attach(m_server, nullptr);
    if (m_attachId == 0)
    {
        g_object_unref(m_server);
        m_server = nullptr;
        setStatusMessage(QStringLiteral("Failed to attach RTSP server"));
        return;
    }

    setStatusMessage(QStringLiteral("Listening rtsp://")
           + (bindAddress.isEmpty() ? QStringLiteral("0.0.0.0") : bindAddress)
           + QLatin1Char(':') + QLatin1String(rtspService)
           + QLatin1String(rtspMount));
}

void WFDServer::handleClientConnected(GstRTSPClient *client)
{
    if (m_session) {
        delete m_session;
    }

    m_session = new WfdClientSession(client, m_bindAddress, this);

    connect(m_session, &WfdClientSession::disconnected, this, [this]() {
        setStatusMessage(QStringLiteral("Sink Disconnected"));
    });

    connect(m_session, &WfdClientSession::playRequested, this, [this](const QString &sinkIp, quint16 sinkPort) {
        setStatusMessage(QStringLiteral("Streaming active to %1:%2").arg(sinkIp).arg(sinkPort));
        Q_EMIT playRequested(sinkIp, sinkPort);
    });

    setStatusMessage(QStringLiteral("Sink Connected, negotiating WFD..."));
}

void WFDServer::stop()
{
    if (m_attachId != 0)
    {
        g_source_remove(m_attachId);
        m_attachId = 0;
    }
    if (m_server)
    {
        g_object_unref(m_server);
        m_server = nullptr;
    }
    if (m_session)
    {
        delete m_session;
        m_session = nullptr;
    }
    setStatusMessage(QStringLiteral("WFD server stopped"));
}

void WFDServer::setStatusMessage(const QString &statusMessage)
{
    if (statusMessage == m_statusMessage) {
        return;
    }
    m_statusMessage = statusMessage;
    Q_EMIT statusMessageChanged();
}

