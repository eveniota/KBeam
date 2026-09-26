//
// Created by mradu1 on 9/14/26.
//

#include "wfdclientsession.h"

#include <QDebug>
#include <QTimer>
#include <gst/gst.h>
#include <gst/rtsp/gstrtspmessage.h>
#include <gst/rtsp-server/rtsp-client.h>

namespace
{
    constexpr int SettleTimeoutMs = 500;
    constexpr char WfdRequireHeader[] = "org.wfa.wfd1.0";
    constexpr char WfdPublicHeader[] = "org.wfa.wfd1.0, GET_PARAMETER, SET_PARAMETER";
    constexpr char WfdContentTypeParameters[] = "text/parameters";
    constexpr char WfdUrl[] = "rtsp://localhost/wfd1.0";

    constexpr char M3RequestBody[] =
        "wfd_video_formats\r\n"
        "wfd_audio_codecs\r\n"
        "wfd_client_rtp_ports\r\n";

    constexpr char DefaultH264Descriptor[] =
        "00 00 01 01 00000080 00000000 00000000 00 0000 0000 00 none none";
}

WfdClientSession::WfdClientSession(GstRTSPClient *client, const QString &serverAddress, QObject *parent)
    : QObject(parent)
    , m_client(GST_RTSP_CLIENT(g_object_ref(client)))
    , m_serverAddress(serverAddress.isEmpty() ? QStringLiteral("127.0.0.1") : serverAddress)
{
    m_closedHandlerId = g_signal_connect(
            m_client, "closed",
            G_CALLBACK(+[](GstRTSPClient *, gpointer userData) {
                static_cast<WfdClientSession *>(userData)->handleClosed();
            }), this);

    m_responseHandlerId = g_signal_connect(
        m_client, "handle-response",
        G_CALLBACK(+[](GstRTSPClient *, GstRTSPContext *ctx, gpointer userData) {
            static_cast<WfdClientSession *>(userData)->handleResponse(ctx);
        }), this);

    m_optionsHandlerId = g_signal_connect(
        m_client, "options-request",
        G_CALLBACK(+[](GstRTSPClient *, GstRTSPContext *ctx, gpointer userData) {
            static_cast<WfdClientSession *>(userData)->handleOptionsRequest(ctx);
        }), this);

    m_playHandlerId = g_signal_connect(
        m_client, "play-request",
        G_CALLBACK(+[](GstRTSPClient *, GstRTSPContext *ctx, gpointer userData)
        {
            static_cast<WfdClientSession *>(userData)->handlePlayRequest(ctx);
        }), this);

    qDebug() << "KCast: RTSP sink connected. Waiting" << SettleTimeoutMs << "ms before M1...";
    QTimer::singleShot(SettleTimeoutMs, this, &WfdClientSession::sendM1Options);
}

WfdClientSession::~WfdClientSession()
{
    if (m_client)
    {
        if (m_closedHandlerId)
        {
            g_signal_handler_disconnect(m_client, m_closedHandlerId);
        }
        if (m_responseHandlerId)
        {
            g_signal_handler_disconnect(m_client, m_responseHandlerId);
        }
        if (m_optionsHandlerId)
        {
            g_signal_handler_disconnect(m_client, m_optionsHandlerId);
        }
        if (m_playHandlerId)
        {
            g_signal_handler_disconnect(m_client, m_playHandlerId);
        }

        g_object_unref(m_client);
        m_client = nullptr;
    }
}

WfdClientSession::State WfdClientSession::state() const
{
    return m_state;
}

quint16 WfdClientSession::sinkRtpPort() const
{
    return m_sinkRtpPort;
}

void WfdClientSession::setState(State state)
{
    if (m_state == state)
    {
        return;
    }
    m_state = state;
    Q_EMIT stateChanged(m_state);
}

void WfdClientSession::sendM1Options()
{
    if (!m_client) {
        return;
    }

    qDebug() << "KCast: Sending WFD M1 Options query..";

    GstRTSPMessage msg;
    memset(&msg, 0, sizeof(msg));

    gst_rtsp_message_init_request(&msg, GST_RTSP_OPTIONS, "*");
    gst_rtsp_message_add_header(&msg, GST_RTSP_HDR_REQUIRE, WfdRequireHeader);

    const GstRTSPResult result = gst_rtsp_client_send_message(m_client, nullptr, &msg);
    gst_rtsp_message_unset(&msg);

    if (result == GST_RTSP_OK)
    {
        setState(State::M1Sent);
        qDebug() << "KCast: M1 options query successfully transmitted";
    } else
    {
        qWarning() << "KCast: M1 options query failed" << result;
    }
}

void WfdClientSession::sendM3GetParameters()
{
    if (!m_client) {
        return;
    }

    qDebug() << "KCast: Sending WFD M3 GET_PARAMETER query...";

    GstRTSPMessage msg;
    memset(&msg, 0, sizeof(msg));

    gst_rtsp_message_init_request(&msg, GST_RTSP_GET_PARAMETER, WfdUrl);
    gst_rtsp_message_add_header(&msg, GST_RTSP_HDR_CONTENT_TYPE, WfdContentTypeParameters);
    gst_rtsp_message_set_body(&msg, reinterpret_cast<const guint8 *>(M3RequestBody),
strlen(M3RequestBody));

    const GstRTSPResult result = gst_rtsp_client_send_message(m_client, nullptr, &msg);
    gst_rtsp_message_unset(&msg);

    if (result == GST_RTSP_OK) {
        setState(State::M3Sent);
        qDebug() << "KCast: M3 GET_PARAMETER transmitted.";
    } else {
        qWarning() << "KCast: Failed to send M3 GET_PARAMETER, error:" << result;
    }
}

void WfdClientSession::sendM4SetParameter()
{
    if (!m_client || m_sinkRtpPort == 0) {
        return;
    }

    qDebug() << "KCast: Sending WFD M4 SET_PARAMETER...";

    const QString presentationUrl = QStringLiteral("rtsp://%1:7236/wfd1.0/streamid=0 none").
arg(m_serverAddress);
    const QString rtpPorts = QStringLiteral("RTP/AVP/UDP;unicast %1 0 mode=play").arg(m_sinkRtpPort);

    const QString body = QStringLiteral(
        "wfd_video_formats: %1\r\n"
        "wfd_audio_codecs: none\r\n"
        "wfd_presentation_URL: %2\r\n"
        "wfd_client_rtp_ports: %3\r\n")
        .arg(QLatin1String(DefaultH264Descriptor))
        .arg(presentationUrl)
        .arg(rtpPorts);

    const QByteArray utf8Body = body.toUtf8();

    GstRTSPMessage msg;
    memset(&msg, 0, sizeof(msg));

    gst_rtsp_message_init_request(&msg, GST_RTSP_SET_PARAMETER, WfdUrl);
    gst_rtsp_message_add_header(&msg, GST_RTSP_HDR_CONTENT_TYPE, WfdContentTypeParameters);
    gst_rtsp_message_set_body(&msg, reinterpret_cast<const guint8 *>(utf8Body.constData()), utf8Body.size());

    const GstRTSPResult result = gst_rtsp_client_send_message(m_client, nullptr, &msg);
    gst_rtsp_message_unset(&msg);

    if (result == GST_RTSP_OK) {
        setState(State::M4Sent);
        qDebug() << "KCast: M4 SET_PARAMETER transmitted.";
    } else {
        qWarning() << "KCast: Failed to send M4 SET_PARAMETER, error:" << result;
    }
}

void WfdClientSession::sendM5TriggerSetup()
{
    if (!m_client) {
        return;
    }

    qDebug() << "KCast: Sending WFD M5 trigger SETUP...";

    constexpr char M5Body[] = "wfd_trigger_method: SETUP\r\n";

    GstRTSPMessage msg;
    memset(&msg, 0, sizeof(msg));

    gst_rtsp_message_init_request(&msg, GST_RTSP_SET_PARAMETER, WfdUrl);
    gst_rtsp_message_add_header(&msg, GST_RTSP_HDR_CONTENT_TYPE, WfdContentTypeParameters);
    gst_rtsp_message_set_body(&msg, reinterpret_cast<const guint8 *>(M5Body), strlen(M5Body));

    const GstRTSPResult result = gst_rtsp_client_send_message(m_client, nullptr, &msg);
    gst_rtsp_message_unset(&msg);

    if (result == GST_RTSP_OK) {
        setState(State::M5Sent);
        qDebug() << "KCast: M5 trigger SETUP transmitted.";
    } else {
        qWarning() << "KCast: Failed to send M5 trigger SETUP, error:" << result;
    }
}


void WfdClientSession::handleClosed()
{
    qDebug() << "KCast: Client closed RTSP connection.";
    Q_EMIT disconnected();
}

void WfdClientSession::handleResponse(GstRTSPContext *ctx)
{
    if (!ctx || !ctx->response)
    {
        return;
    }

    GstRTSPStatusCode statusCode = GST_RTSP_STS_INVALID;
    gst_rtsp_message_parse_response(ctx->response, &statusCode, nullptr, nullptr);

    qDebug() << "KCast: Received RTSP response with status code:" << statusCode << "in state:" << static_cast<int>(m_state);
    if (statusCode != GST_RTSP_STS_OK)
    {
        qWarning() << "KCast: Sink Replied with non-200 status code:" << statusCode;
        return;
    }

    if (m_state == State::M1Sent)
    {
        qDebug() << "KCast: Sink accepted M1. Advancing to M3 GET_PARAMETER...";
        sendM3GetParameters();
    } else if (m_state == State::M3Sent)
    {
        guint8 *bodyData = nullptr;
        guint bodySize = 0;
        gst_rtsp_message_get_body(ctx->response, &bodyData, &bodySize);

        if (bodyData && bodySize > 0)
        {
            const QString body = QString::fromUtf8(reinterpret_cast<const char *>(bodyData), static_cast<int>(bodySize));
            parseM3Response(body);
        } else
        {
            qWarning() << "KCast: Received empty body in M3 Response";
        }
    } else if (m_state == State::M4Sent)
    {
        qDebug() << "KCast: Sink accepted M4 parameters! Sending M5 trigger SETUP...";
        sendM5TriggerSetup();
    } else if (m_state == State::M5Sent)
    {
        qDebug() << "KCast: Sink acknowledged M5! Waiting for sink RTSP SETUP and PLAY...";
    }
}

void WfdClientSession::parseM3Response(const QString &body)
{
    qDebug() << "KCast: Parsing M3 response body:\n" << body;

    const auto lines = QStringView(body).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const auto &line : lines) {
        const auto trimmed = line.trimmed();
        if (trimmed.startsWith(QLatin1String("wfd_client_rtp_ports:"))) {
            // Expected format: wfd_client_rtp_ports: RTP/AVP/UDP;unicast <primaryPort> <secondaryPort> mode=play
            const auto tokens = trimmed.split(QLatin1Char(' '), Qt::SkipEmptyParts);
            if (tokens.size() >= 3) {
                bool ok = false;
                const quint16 port = tokens.at(2).toUShort(&ok);
                if (ok && port > 0) {
                    m_sinkRtpPort = port;
                    qDebug() << "KCast: Successfully negotiated sink RTP port:" << m_sinkRtpPort;
                    Q_EMIT rtpPortNegotiated(m_sinkRtpPort);

                    sendM4SetParameter();
                }
            }
        }
    }
}

void WfdClientSession::handleOptionsRequest(GstRTSPContext *ctx)
{
    if (!ctx || !ctx->response) {
        return;
    }

    gst_rtsp_message_add_header(ctx->response, GST_RTSP_HDR_PUBLIC, WfdPublicHeader);
    qDebug() << "KCast: Handled M2 Options request, added Public:" << WfdPublicHeader;
}

void WfdClientSession::handlePlayRequest(GstRTSPContext *ctx)
{
    Q_UNUSED(ctx);
    qDebug() << "KCast: Sink issued PLAY! Transitioning to Streaming state.";
    setState(State::Streaming);

    QString sinkIp;
    if (m_client) {
        GstRTSPConnection *conn = gst_rtsp_client_get_connection(m_client);
        if (conn) {
            const gchar *ip = gst_rtsp_connection_get_ip(conn);
            if (ip) {
                sinkIp = QString::fromUtf8(ip);
            }
        }
    }
    if (sinkIp.isEmpty()) {
        sinkIp = QStringLiteral("127.0.0.1");
    }

    qDebug() << "KCast: Ready to stream RTP to sink:" << sinkIp << ":" << m_sinkRtpPort;
    Q_EMIT playRequested(sinkIp, m_sinkRtpPort);
}
