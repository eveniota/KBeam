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
    KLocalizedString::setApplicationDomain("kcast");
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));

    qmlRegisterSingletonInstance("org.kde.kcast", 1, 0, "P2PDiscovery", &discovery);

    ScreencastPortal screencast;
    qmlRegisterSingletonInstance("org.kde.kcast", 1, 0, "ScreencastPortal", &screencast);

    StreamPipeline pipeline;
    qmlRegisterSingletonInstance("org.kde.kcast", 1, 0, "StreamPipeline", &pipeline);

    WFDServer wfdServer;
    qmlRegisterSingletonInstance("org.kde.kcast", 1, 0, "WFDServer", &wfdServer);

    // Track state between portal capture (fd, nodeId) and RTSP negotiation (sinkIp, sinkPort)
    struct StreamingState {
        int fd = -1;
        uint nodeId = 0;
        QString sinkIp;
        quint16 sinkPort = 0;
    } session;

    auto tryStartPipeline = [&pipeline, &session]() {
        if (session.fd >= 0 && session.sinkPort > 0) {
            const QString targetHost = session.sinkIp.isEmpty() ? QStringLiteral("127.0.0.1") : session.sinkIp;
            qDebug() << "KCast: Starting stream pipeline to" << targetHost << ":" << session.sinkPort;
            pipeline.start(session.fd, session.nodeId, targetHost, session.sinkPort);
        }
    };

    // When ScreencastPortal yields PipeWire fd and nodeId:
    QObject::connect(&screencast, &ScreencastPortal::started,
        [&session, tryStartPipeline](int fd, uint nodeId) {
            session.fd = fd;
            session.nodeId = nodeId;
            tryStartPipeline();
        });

    // When sink sends RTSP PLAY:
    QObject::connect(&wfdServer, &WFDServer::playRequested,
        [&session, &screencast, tryStartPipeline](const QString &sinkIp, quint16 sinkPort) {
            session.sinkIp = sinkIp;
            session.sinkPort = sinkPort;

            // If user hasn't selected a screen yet, prompt portal now
            if (session.fd < 0) {
                screencast.start();
            } else {
                tryStartPipeline();
            }
        });

    // Dynamic RTSP server binding: bind to P2P IP when connected, stop when disconnected
    QObject::connect(&discovery, &P2PDiscovery::ipv4AddressChanged,
        [&discovery, &wfdServer, &pipeline, &session]() {
            const QString ip = discovery.ipv4Address();
            if (!ip.isEmpty()) {
                qDebug() << "KCast: Binding WFD RTSP server to P2P IP:" << ip;
                wfdServer.start(ip);
            } else {
                qDebug() << "KCast: P2P disconnected, stopping server and pipeline.";
                pipeline.stop();
                wfdServer.stop();
                session.fd = -1;
                session.sinkPort = 0;
            }
        });

    // Fallback: start on localhost for local testing (ffplay / VLC)
    wfdServer.start(QStringLiteral("127.0.0.1"));

    engine.loadFromModule("org.kde.kcast", "Main");
    return app.exec();
}