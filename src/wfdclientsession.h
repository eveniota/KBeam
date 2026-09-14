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

Q_SIGNALS:
    void stateChanged(State state);
    void disconnected();
    void rtpPortNegotiated(quint16 rtpPort);

public Q_SLOTS:
    void sendM1Options();

private:
    void setState(State state);

    static void onClosedBridge(GstRTSPClient *client, gpointer userData);
    static void onHandleResponseBridge(GstRTSPClient *client, GstRTSPContext *ctx, gpointer userData);
    static void onOptionsRequestBridge(GstRTSPClient *client, GstRTSPContext *context, gpointer userData);

    void handleClosed();
    void handleResponse(GstRTSPContext *ctx);
    void handleOptionsRequest(GstRTSPContext *ctx);
    void sendM3GetParameters();

    GstRTSPClient *m_client = nullptr;
    State m_state = State::Init;

    gulong m_closedHandlerId = 0;
    gulong m_responseHandlerId = 0;
    gulong m_optionsHandlerId = 0;
};


