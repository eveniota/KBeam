//
// Created by mradu1 on 8/2/26.
//

#include "screencastportal.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusObjectPath>
#include <QRandomGenerator>

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
    setStatusMessage(QStringLiteral("Creating screencast session..."));
    createSession();
}

QString ScreencastPortal::makeToken(const QString &prefix) const
{
    return prefix + QString::number(QRandomGenerator::global()->generate());
}

void ScreencastPortal::createSession() {
    QDBusInterface portal (
        QStringLiteral("org.freedesktop.portal.Desktop"),
        QStringLiteral("/org/freedesktop/portal/desktop"),
        QStringLiteral("org.freedesktop.portal.ScreenCast"),
        QDBusConnection::sessionBus());

    const QString requestToken = makeToken(QStringLiteral("kcast_req_"));
    const QString sessionToken = makeToken(QStringLiteral("kcast_sess_"));

    const QVariantMap options = {
        {QStringLiteral("handle_token"), requestToken},
        {QStringLiteral("session_handle_token"), sessionToken},
    };

    const QDBusReply<QDBusObjectPath> reply = portal.call(QStringLiteral("CreateSession"), options);

    if (!reply.isValid())
    {
        setStatusMessage(reply.error().message());
        Q_EMIT failed(reply.error().message());
        return;
    }
    const QString requestPath = reply.value().path();
    const bool ok = QDBusConnection::sessionBus().connect(
        QString(),
        requestPath,
        QStringLiteral("org.freedesktop.portal.Request"),
        QStringLiteral("Response"),
        this,
        SLOT(onCreateSessinResponse(uint, QVariantMap)));

    if (!ok)
    {
        setStatusMessage(QStringLiteral("Failed to listen for createSession response"));
        Q_EMIT failed(reply.error().message());
    }
}

void ScreencastPortal::onCreateSessionResponse(uint response, const QVariantMap &results)
{
    if (response == 0)
    {
        setStatusMessage(QStringLiteral("CreateSession cancelled or failed"));
        Q_EMIT failed(QStringLiteral("CreateSession cancelled or failed"));
        return;
    }

    m_sessionPath = results.value(QStringLiteral("session_handle")).value<QDBusObjectPath>();
    setStatusMessage(QStringLiteral("Session ready: ") + m_sessionPath.path());
}




