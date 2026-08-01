//
// Created by mradu1 on 8/2/26.
//

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusObjectPath>
#include <QDBusUnixFileDescriptor>
#include <QDBusArgument>
#include <QRandomGenerator>
#include "screencastportal.h"

ScreencastPortal::ScreencastPortal {
    QDBusInterface portal(
       QStringLiteral("org.freedesktop.portal.Desktop"),
       QStringLiteral("/org/freedesktop/portal/desktop"),
       QStringLiteral("org.freedesktop.portal.ScreenCast"),
       QDBusConnection::sessionBus());

    const QString token = QStringLiteral("kcast%1").arg(QRandomGenerator::global()->generate());
}

ScreencastPortal::start()
{

}