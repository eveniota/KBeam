//
// Created by mradu1 on 9/14/26.
//
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
        M1Sent,
        M2Received,
        M3Sent,
        M4Sent,
        M5Sent,
        Streaming,
    };
    Q_ENUM(State)

    explicit WfdClientSession(GstRTSPClient *client, QObject *parent = nullptr);
    ~WfdClientSession() override;

    State state() const;
    quint16 sinkRtpPort() const;

Q_SIGNALS:
    void stateChanged(State state);
    void disconnected();
    void rtpPortNegotiated(quint16 rtpPort);

public Q_SLOTS:
    void sendM1Options();
    void sendM3GetParameters();

private:
    void setState(State state);
    void handleClosed();
    void handleResponse(GstRTSPContext *ctx);
    void handleOptionsRequest(GstRTSPContext *ctx);
    void parseM3Response(const QString &body);

    GstRTSPClient *m_client = nullptr;
    State m_state = State::Init;
    quint16 m_sinkRtpPort = 0;

    gulong m_closedHandlerId = 0;
    gulong m_responseHandlerId = 0;
    gulong m_optionsHandlerId = 0;
};


