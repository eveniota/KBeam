//
// Created by mradu1 on 8/8/26.
//
#include "streampipeline.h"

StreamPipeline::StreamPipeline(QObject *parent) : QObject(parent), m_pipeline(nullptr) {}

QString StreamPipeline::statusMessage() const{
    return m_statusMessage;
}

void StreamPipeline::setStatusMessage(const QString &statusMessage) {
    if (statusMessage == m_statusMessage) {
        return;
    }
    m_statusMessage = statusMessage;
    Q_EMIT statusMessageChanged();
}

void StreamPipeline::start(int fd, uint nodeId)
{
    setStatusMessage(QStringLiteral("Pipeline ready fd=") + QString::number(fd)
       + QStringLiteral(" node=") + QString::number(nodeId));  
}

void StreamPipeline::stop()
{
    setStatusMessage(QStringLiteral("stop"));
}



