// SPDX-FileCopyrightText: 2026 Mradul Pal <mradulpal@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QDebug>
#include <QQmlContext>
#include <QQmlEngine>
#include <KLocalizedString>
#include <KLocalizedContext>
#include "p2pdiscovery.h"
#include "screencastportal.h"
#include "streampipeline.h"
#include "wfdserver.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));

    P2PDiscovery discovery;
    QQmlApplicationEngine engine;
    KLocalizedString::setApplicationDomain("kbeam");
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));

    qmlRegisterSingletonInstance("org.kde.kbeam", 1, 0, "P2PDiscovery", &discovery);

    ScreencastPortal screencast;
    qmlRegisterSingletonInstance("org.kde.kbeam", 1, 0, "ScreencastPortal", &screencast);

    StreamPipeline pipeline;
    qmlRegisterSingletonInstance("org.kde.kbeam", 1, 0, "StreamPipeline", &pipeline);

    WFDServer wfdServer;
    qmlRegisterSingletonInstance("org.kde.kbeam", 1, 0, "WFDServer", &wfdServer);

    // Track state between portal capture (fd, nodeId) and RTSP negotiation (sinkIp, sinkPort)
    struct StreamingState {
        int fd = -1;
    } session;

    // When ScreencastPortal yields PipeWire fd and nodeId:
    QObject::connect(&screencast, &ScreencastPortal::started,
        [&pipeline, &session](int fd, uint nodeId) {
            session.fd = fd;
            qDebug() << "KBeam: Screen selected (fd=" << fd << ", node=" << nodeId << "). Startingdesktop capture pipeline...";
            pipeline.start(fd, nodeId);
        });

    // When sink sends RTSP PLAY:
    QObject::connect(&wfdServer, &WFDServer::playRequested,
        [&session, &screencast]() {
            // If user hasn't selected a screen yet, prompt portal now
            if (session.fd < 0) {
                screencast.start();
            }
        });

    wfdServer.start(QStringLiteral("0.0.0.0"));

    // Update RTSP presentation address when P2P IP is assigned; stop active pipeline when disconnected
    QObject::connect(&discovery, &P2PDiscovery::ipv4AddressChanged,
        [&discovery, &wfdServer, &pipeline, &session]() {
            const QString ip = discovery.ipv4Address();
            if (!ip.isEmpty()) {
                qDebug() << "KBeam: P2P connected with IP:" << ip;
                wfdServer.setServerAddress(ip);
            } else {
                qDebug() << "KBeam: P2P disconnected, stopping active pipeline.";
                pipeline.stop();
                session.fd = -1;
            }
        });

    engine.loadFromModule("org.kde.kbeam", "Main");
    return app.exec();
}