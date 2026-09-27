//
// Created by mradu1 on 8/8/26.
//

#include "streampipeline.h"

#include <QDebug>
#include <gst/gst.h>

namespace
{
constexpr char DefaultHost[] = "127.0.0.1";
constexpr quint16 DefaultPort = 5000;
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

void StreamPipeline::start(int fd, uint nodeId, const QString &destinationHost, quint16 destinationPort)
{
    Q_UNUSED(destinationHost);
    Q_UNUSED(destinationPort);
    stop();

    const QString pipelineDesc =
        QStringLiteral(
            "pipewiresrc fd=%1 path=%2 do-timestamp=true keepalive-time=1000 "
            "! videoconvert "
            "! videorate "
            "! video/x-raw,format=I420 "
            "! intervideosink channel=kbeam-desktop")
            .arg(fd)
            .arg(nodeId);

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

    const GstStateChangeReturn ret = gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        qWarning() << "KBeam: Failed to set capture pipeline to PLAYING";
        setStatusMessage(QStringLiteral("Failed to start capture pipeline"));
        return;
    }
    setStatusMessage(QStringLiteral("Desktop capture active (fd=%1, node=%2)").arg(fd).arg(nodeId));
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