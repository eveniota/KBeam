//
// Created by mradu1 on 8/10/26.
//

#pragma once
#include <gst/rtsp-server/rtsp-client.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(KcastRtspClient, kcast_rtsp_client, KCAST, RTSP_CLIENT, GstRTSPClient)

KcastRtspClient *kcast_rtsp_client_new(void);
void kcast_rtsp_client_query_support(KcastRtspClient *self);

G_END_DECLS

