//
// Created by mradu1 on 8/8/26.
//
#include "streampipeline.h"

#include <gst/gst.h>

namespace
{
constexpr char rtpHost[] = "127.0.0.1";
constexpr int rtpPort = 5000;
}

StreamPipeline::StreamPipeline(QObject *parent)
   : QObject(parent)
   , m_pipeline(nullptr)
{
   gst_init(nullptr, nullptr);
}

QString StreamPipeline::statusMessage() const
{
   return m_statusMessage;
}

void StreamPipeline::setStatusMessage(const QString &statusMessage)
{
   if (m_statusMessage == statusMessage) {
       return;
   }
   m_statusMessage = statusMessage;
   Q_EMIT statusMessageChanged();
}

void StreamPipeline::start(int fd, uint nodeId)
{
   stop();

   const QString pipelineDesc =
       QStringLiteral(
           "pipewiresrc fd=%1 path=%2 do-timestamp=true "
           "! videoconvert "
           "! video/x-raw,format=I420 "
           "! x264enc tune=zerolatency speed-preset=ultrafast key-int-max=30 "
           "! video/x-h264,stream-format=byte-stream "
           "! mpegtsmux alignment=7 "
           "! rtpmp2tpay "
           "! udpsink host=%3 port=%4")
           .arg(fd)
           .arg(nodeId)
           .arg(QLatin1String(rtpHost))
           .arg(rtpPort);

   GError *error = nullptr;
   m_pipeline = gst_parse_launch(pipelineDesc.toUtf8().constData(), &error);
   if (!m_pipeline) {
       const QString message = error
           ? QString::fromUtf8(error->message)
           : QStringLiteral("gst_parse_launch failed");
       g_clear_error(&error);
       setStatusMessage(message);
       return;
   }

   gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
   setStatusMessage(QStringLiteral("RTP ") + QLatin1String(rtpHost)
       + QLatin1Char(':') + QString::number(rtpPort)
       + QStringLiteral(" fd=") + QString::number(fd)
       + QStringLiteral(" node=") + QString::number(nodeId));
}

void StreamPipeline::stop()
{
   if (!m_pipeline) {
       setStatusMessage(QStringLiteral("Stopped"));
       return;                                                                                     
   }

   gst_element_set_state(m_pipeline, GST_STATE_NULL);
   gst_object_unref(m_pipeline);
   m_pipeline = nullptr;
   setStatusMessage(QStringLiteral("Stopped"));
}