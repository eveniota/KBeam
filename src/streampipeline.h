//
// Created by mradu1 on 8/4/26.
//

#pragma once
    
#include <QObject>
#include <QString>

typedef struct _GstElement GstElement;

class StreamPipeline : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
    explicit StreamPipeline(QObject *parent = nullptr);
    QString statusMessage() const;

    Q_INVOKABLE void start(int fd,
                           uint nodeId,
                           const QString &destinationHost = QStringLiteral("127.0.0.1"),
                           quint16 destinationPort = 5000);

    Q_INVOKABLE void stop();

    Q_SIGNALS:
        void statusMessageChanged();

private:
    void setStatusMessage(const QString &statusMessage);

    GstElement *m_pipeline = nullptr;
    QString m_statusMessage;
};
