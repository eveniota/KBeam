//
// Created by mradu1 on 7/25/26.
//
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QDebug>
#include <QQmlContext>
#include <KLocalizedContext>
#include <KLocalizedString>
#include "p2pdiscovery.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    P2PDiscovery discovery;
    QQmlApplicationEngine engine;
    KLocalizedString::setApplicationDomain("kcast");
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.rootContext()->setContextProperty(QStringLiteral("p2p"), &discovery);
    engine.loadFromModule("org.kde.kcast", "Main");
    return app.exec();
}
