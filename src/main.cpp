//
// Created by mradu1 on 7/25/26.
//
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
        uint nodeId = 0;
        QString sinkIp;
        quint16 sinkPort = 0;
    } session;

    // When ScreencastPortal yields PipeWire fd and nodeId:
    QObject::connect(&screencast, &ScreencastPortal::started,
        [&pipeline, &session](int fd, uint nodeId) {
            session.fd = fd;
            session.nodeId = nodeId;
            qDebug() << "KBeam: Screen selected (fd=" << fd << ", node=" << nodeId << "). Startingdesktop capture pipeline...";
            pipeline.start(fd, nodeId);
        });

    // When sink sends RTSP PLAY:
    QObject::connect(&wfdServer, &WFDServer::playRequested,
        [&session, &screencast](const QString &sinkIp, quint16 sinkPort) {
            session.sinkIp = sinkIp;
            session.sinkPort = sinkPort;

            // If user hasn't selected a screen yet, prompt portal now
            if (session.fd < 0) {
                screencast.start();
            }
        });

    // Dynamic RTSP server binding: bind to P2P IP when connected, stop when disconnected
    QObject::connect(&discovery, &P2PDiscovery::ipv4AddressChanged,
        [&discovery, &wfdServer, &pipeline, &session]() {
            const QString ip = discovery.ipv4Address();
            if (!ip.isEmpty()) {
                qDebug() << "KBeam: Binding WFD RTSP server to P2P IP:" << ip;
                wfdServer.start(ip);
            } else {
                qDebug() << "KBeam: P2P disconnected, stopping server and pipeline.";
                pipeline.stop();
                wfdServer.stop();
                session.fd = -1;
                session.sinkPort = 0;
            }
        });

    // Fallback: start on localhost for local testing (ffplay / VLC)
    wfdServer.start(QStringLiteral("0.0.0.0"));

    engine.loadFromModule("org.kde.kbeam", "Main");
    return app.exec();
}