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
}

WfdClientSession::WfdClientSession(GstRTSPClient *client, QObject *parent)
    : QObject(parent)
    , m_client(GST_RTSP_CLIENT(g_object_ref(client)))
{
    m_closedHandlerId = g_signal_connect(
        m_client, "closed", G_CALLBACK(onClosedBridge), this);

    m_responseHandlerId = g_signal_connect(
        m_client, "handle-response", G_CALLBACK(onHandleResponseBridge), this);

    m_optionsHandlerId = g_signal_connect(
        m_client, "options-request", G_CALLBACK(onOptionsRequestBridge), this);

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

    qDebug() << "KCast: Received RTSP response with status code: " << statusCode;
    if (m_state == State::M1Sent && statusCode == GST_RTSP_STS_OK)
    {
        qDebug() << "KCast: Sink Accepted M1! Ready for M3 Parameter Exchange.";
    }
}

void WfdClientSession::handleOptionsRequest(GstRTSPContext *ctx)
{
    if (!ctx || !ctx->response) {
        return;
    }

    gst_rtsp_message_add_header(ctx->response, GST_RTSP_HDR_PUBLIC, WfdPublicHeader);
    qDebug() << "KCast: Handled M2 Options request, added PUblic:" << WfdPublicHeader;
}

void WfdClientSession::onClosedBridge(GstRTSPClient *client, gpointer userData)
{
    Q_UNUSED(client);
    auto *session = static_cast<WfdClientSession *>(userData);
    session->handleClosed();
}

void WfdClientSession::onHandleResponseBridge(GstRTSPClient *client, GstRTSPContext *ctx, gpointer userData)
{
    Q_UNUSED(client);
    auto *session = static_cast<WfdClientSession *>(userData);
    session->handleResponse(ctx);
}

void WfdClientSession::onOptionsRequestBridge(GstRTSPClient *client, GstRTSPContext *ctx, gpointer userData)
{
    Q_UNUSED(client);
    auto *session = static_cast<WfdClientSession *>(userData);
    session->handleOptionsRequest(ctx);
}