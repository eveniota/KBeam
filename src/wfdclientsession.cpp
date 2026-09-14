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
}

WfdClientSession::WfdClientSession(GstRTSPClient *client, QObject *parent)
    : QObject(parent)
    , m_client(GST_RTSP_CLIENT(g_object_ref(client)))
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