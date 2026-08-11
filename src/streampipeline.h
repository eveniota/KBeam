//
// Created by mradu1 on 8/4/26.
//

#pragma once
#include <QObject>
#include <QString>

typedef struct _GstElement GstElement;

class StreamPipeline: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
    explicit StreamPipeline(QObject *parent = nullptr);
    QString statusMessage() const;
    Q_INVOKABLE void start(int fd, uint nodeId);
    Q_INVOKABLE void stop();

Q_SIGNALS:
    void statusMessageChanged();

private:
    void setStatusMessage(const QString& statusMessage);
    GstElement* m_pipeline;
    QString m_statusMessage;
};
