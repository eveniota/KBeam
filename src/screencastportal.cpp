//
// Created by mradu1 on 8/2/26.
//

#include "screencastportal.h"

ScreencastPortal::ScreencastPortal (QObject *parent)
    : QObject(parent)
{}


QString ScreencastPortal::statusMessage() const
{
    return m_statusMessage;
}

void ScreencastPortal::setStatusMessage(const QString &statusMessage)
{
    if (m_statusMessage == statusMessage)
    {
        return;
    }
    m_statusMessage = statusMessage;
    Q_EMIT statusMessageChanged();
}

void ScreencastPortal::start()
{
    setStatusMessage(QStringLiteral("ScreenCast is not implemented yet"));
}


