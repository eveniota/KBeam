//
// Created by mradu1 on 7/25/26.
//
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    QQmlApplicationEngine engine;
    engine.loadFromModule("org.kde.kcast", "Main");
    return app.exec();
}
