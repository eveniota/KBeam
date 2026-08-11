//
// Created by mradu1 on 8/2/26.
//

#include "screencastportal.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusObjectPath>
#include <QRandomGenerator>
#include <QDBusArgument>
#include <QDBusUnixFileDescriptor>

struct PortalStream {
       uint nodeId = 0;
       QVariantMap options;
   };

   QDBusArgument &operator<<(QDBusArgument &argument, const PortalStream &stream)
   {
       argument.beginStructure();
       argument << stream.nodeId << stream.options;
       argument.endStructure();
       return argument;
   }

   const QDBusArgument &operator>>(const QDBusArgument &argument, PortalStream &stream)
   {
       argument.beginStructure();
       argument >> stream.nodeId >> stream.options;
       argument.endStructure();
       return argument;
   }

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
        SLOT(onCreateSessionResponse(uint,QVariantMap)));

    if (!ok)
    {
        setStatusMessage(QStringLiteral("Failed to listen for createSession response"));
        Q_EMIT failed(QStringLiteral("Failed to listen for createSession response"));
    }
}

void ScreencastPortal::onCreateSessionResponse(uint response, const QVariantMap &results)
{
    if (response != 0)
    {
        setStatusMessage(QStringLiteral("CreateSession cancelled or failed"));
        Q_EMIT failed(QStringLiteral("CreateSession cancelled or failed"));
        return;
    }

    const QVariant handle = results.value(QStringLiteral("session_handle"));
    if (handle.canConvert<QDBusObjectPath>()) {
        m_sessionPath = handle.value<QDBusObjectPath>();
    } else {
        m_sessionPath = QDBusObjectPath(handle.toString());
    }
    if (m_sessionPath.path().isEmpty()) {
        setStatusMessage(QStringLiteral("Missing session_handle"));
        Q_EMIT failed(QStringLiteral("Missing session_handle"));
        return;
    }
    selectSources();
}

void ScreencastPortal::selectSources()
{
    if (m_sessionPath.path().isEmpty())
    {
        setStatusMessage(QStringLiteral("No Session"));
        Q_EMIT failed(QStringLiteral("No Session"));
        return;
    }

    QDBusInterface portal(
        QStringLiteral("org.freedesktop.portal.Desktop"),
        QStringLiteral("/org/freedesktop/portal/desktop"),
        QStringLiteral("org.freedesktop.portal.ScreenCast"),
        QDBusConnection::sessionBus());

    const QString requestToken = makeToken(QStringLiteral("kcast_sel_"));

    const QVariantMap options = {
        {QStringLiteral("handle_token"), requestToken},
        {QStringLiteral("types"), 1u},
        {QStringLiteral("multiple"), false},
        {QStringLiteral("cursor_mode"), 2u},
    };

    const QDBusReply<QDBusObjectPath> reply = portal.call(QStringLiteral("SelectSources"), QVariant::fromValue(m_sessionPath), options);

    if (!reply.isValid())
    {
        setStatusMessage(reply.error().message());
        Q_EMIT failed(reply.error().message());
        return;
    }

    const QString requestPath = reply.value().path();
    QDBusConnection::sessionBus().connect(
           QString(),
           requestPath,
           QStringLiteral("org.freedesktop.portal.Request"),
           QStringLiteral("Response"),
           this,
           SLOT(onSelectSourcesResponse(uint,QVariantMap)));
}

void ScreencastPortal::onSelectSourcesResponse(uint response, const QVariantMap &results)
{
    Q_UNUSED(results);
    if (response != 0)
    {
        setStatusMessage(QStringLiteral("SelectSources cancelled or failed"));
        Q_EMIT failed(QStringLiteral("SelectSources cancelled or failed"));
        return;
    }
    setStatusMessage(QStringLiteral("Sources selected"));
    startSession();
}

void ScreencastPortal::startSession()
{
    if (m_sessionPath.path().isEmpty())
    {
        setStatusMessage(QStringLiteral("No Session"));
        Q_EMIT failed(QStringLiteral("No Session"));
        return;
    }
    QDBusInterface portal(
        QStringLiteral("org.freedesktop.portal.Desktop"),
        QStringLiteral("/org/freedesktop/portal/desktop"),
        QStringLiteral("org.freedesktop.portal.ScreenCast"),
        QDBusConnection::sessionBus());

    const QString requestToken = makeToken(QStringLiteral("kcast_start_"));
    const QVariantMap options = {
        {QStringLiteral("handle_token"), requestToken},
    };

    const QDBusReply<QDBusObjectPath> reply = portal.call(QStringLiteral("Start"),
        QVariant::fromValue(m_sessionPath),
        QString(),
        options);

    if (!reply.isValid())
    {
        setStatusMessage(reply.error().message());
        Q_EMIT failed(reply.error().message());
        return;
    }
    QDBusConnection::sessionBus().connect(
        QString(),
        reply.value().path(),
        QStringLiteral("org.freedesktop.portal.Request"),
        QStringLiteral("Response"),
        this,
        SLOT(onStartResponse(uint,QVariantMap)));
}

void ScreencastPortal::onStartResponse(uint response, const QVariantMap &results)
{
    if (response != 0) {
        setStatusMessage(QStringLiteral("Start cancelled or failed"));
        Q_EMIT failed(QStringLiteral("Start failed"));
        return;
    }
    QDBusArgument arg = results.value(QStringLiteral("streams")).value<QDBusArgument>();

    const QList<PortalStream> streams =
      qdbus_cast<QList<PortalStream>>(results.value(QStringLiteral("streams")));

    if (streams.isEmpty()) {
       setStatusMessage(QStringLiteral("No streams"));
       Q_EMIT failed(QStringLiteral("No streams"));
       return;
    }

    const uint nodeId = streams.constFirst().nodeId;
    setStatusMessage(QStringLiteral("Opening PipeWire remote…"));
    openPipeWireRemote(nodeId);
}

void ScreencastPortal::openPipeWireRemote(uint nodeId)
{
    QDBusInterface portal(
        QStringLiteral("org.freedesktop.portal.Desktop"),
        QStringLiteral("/org/freedesktop/portal/desktop"),
        QStringLiteral("org.freedesktop.portal.ScreenCast"),
        QDBusConnection::sessionBus());

    const QDBusReply<QDBusUnixFileDescriptor> reply =
           portal.call(QStringLiteral("OpenPipeWireRemote"),
                       QVariant::fromValue(m_sessionPath),
                       QVariantMap{});

    if (!reply.isValid()) {
        setStatusMessage(reply.error().message());
        Q_EMIT failed(reply.error().message());
        return;
    }
    const int fd = reply.value().takeFileDescriptor();
    if (fd < 0) {
        setStatusMessage(QStringLiteral("Invalid PipeWire FD"));
        Q_EMIT failed(QStringLiteral("Invalid PipeWire FD"));
        return;
    }

    setStatusMessage(QStringLiteral("Screencast ready (fd=%1, node=%2)")
                         .arg(fd)
                         .arg(nodeId));
    Q_EMIT started(fd, nodeId);
}


