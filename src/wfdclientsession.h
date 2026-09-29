// SPDX-FileCopyrightText: 2026 Mradul Pal <mradulpal@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QString>
#include <glib.h>

typedef struct _GstRTSPClient GstRTSPClient;
typedef struct _GstRTSPContext GstRTSPContext;

class WfdClientSession : public QObject
{
    Q_OBJECT
public:
    enum class State
    {
        Init,
        M1Sent,         // OPTIONS sent
        M3Sent,         // GET_PARAMETER sent
        M4Sent,         // SET_PARAMETER (video/URL/ports) sent
        M5Sent,         // Trigger SETUP sent
        Streaming,      // PLAY request received, ready for media
    };
    Q_ENUM(State)

    explicit WfdClientSession(GstRTSPClient *client, const QString &serverAddress = QString(), QObject *parent = nullptr);
    ~WfdClientSession() override;

    State state() const;
    quint16 sinkRtpPort() const;

Q_SIGNALS:
    void stateChanged(State state);
    void disconnected();
    void playRequested(const QString &sinkIp, quint16 sinkPort);

public Q_SLOTS:
    void sendM1Options();
    void sendM3GetParameters();
    void sendM4SetParameter();
    void sendM5TriggerSetup();
    void sendM16KeepAlive();

private:
    void setState(State state);
    void handleClosed();
    void handleResponse(GstRTSPContext *ctx);
    void handleOptionsRequest(GstRTSPContext *ctx);
    void handlePlayRequest(GstRTSPContext *ctx);
    void parseM3Response(const QString &body);

    class QTimer *m_keepAliveTimer = nullptr;
    GstRTSPClient *m_client = nullptr;
    State m_state = State::Init;
    QString m_serverAddress;
    quint16 m_sinkRtpPort = 0;
    quint16 m_sinkRtcpPort = 0;
    gulong m_closedHandlerId = 0;
    gulong m_responseHandlerId = 0;
    gulong m_optionsHandlerId = 0;
    gulong m_playHandlerId = 0;
};


