//
// Created by mradu1 on 8/10/26.
//

#include "wfdserver.h"
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

void WFDServer::onClientConnectedBridge(GstRTSPServer *server, GstRTSPClient *client, void *userData)
{
    Q_UNUSED(server);
    auto *serverInstance = static_cast<WFDServer *>(userData);
    serverInstance->handleClientConnected(client);
}

void  WFDServer::start(const QString &bindAddress)
{
    stop();
    m_server = gst_rtsp_server_new();

    g_signal_connect(m_server, "client-connected", G_CALLBACK(onClientConnectedBridge), this);
    gst_rtsp_server_set_service(m_server, rtspService);
    if (!bindAddress.isEmpty())
    {
        gst_rtsp_server_set_address(m_server, bindAddress.toUtf8().constData());
    }

    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(m_server);
    GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();

    gst_rtsp_media_factory_set_launch(factory, "( videotestsrc is-live=true ! x264enc ! rtph264pay name=pay0 pt=96 )");
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
    if (m_session)
    {
        delete m_session;
    }
    m_session = new WfdClientSession(client, this);
    connect(m_session, &WfdClientSession::disconnected, this, [this]() {
        setStatusMessage(QStringLiteral("Sink Disconnected"));
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
    if (statusMessage == m_statusMessage)
    {
        return;
    }
    m_statusMessage = statusMessage;
    Q_EMIT statusMessageChanged();
}

