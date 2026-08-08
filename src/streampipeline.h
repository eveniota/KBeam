//
// Created by mradu1 on 8/4/26.
//

#pragma once
#include <QObject>

typedef struct _GstElement GstElement;

class Streampipeline: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged);

public:
    explicit Streampipeline(QObject *parent = nullptr);
    QString statusMessage() const;
    void start(int fd, uint nodeId);
    void stop();

Q_SIGNALS:
    void statusMessageChanged();

private:
    void setStatusMessage(QString message);
    GstElement* m_pipeline;
    QString m_statusMessage;
};
