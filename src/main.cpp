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
    QObject::connect(&screencast, &ScreencastPortal::started, &pipeline, &StreamPipeline::start);
    engine.loadFromModule("org.kde.kcast", "Main");
    return app.exec();
}
