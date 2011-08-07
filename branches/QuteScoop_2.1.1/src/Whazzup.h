/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef WHAZZUP_H_
#define WHAZZUP_H_

#include "_pch.h"

#include "WhazzupData.h"
#include "Window.h"
#include "GuiMessage.h"

class Whazzup: public QObject {
    Q_OBJECT

public:
    static Whazzup* getInstance();

    const WhazzupData& whazzupData() { return (predictedTime.isValid()?
                                                   predictedData: data); } // we fake it when predicting a certain time
    const WhazzupData& realWhazzupData() { return data; } // this is always the really downloaded thing

    void setPredictedTime(QDateTime predictedTime);
    QDateTime getPredictedTime() const { return predictedTime; }

    QString getUserLink(const QString& id) const;
    QString getAtisLink(const QString& id) const;

    QList <QPair <QDateTime, QString> > getDownloadedWhazzups() const;

signals:
    void newData(bool isNew);
    void statusDownloaded();
    void needBookings();

public slots:
    void download();
    void fromFile(QString filename);
    void setStatusLocation(const QString& url);
    void downloadBookings();

private slots:
    void statusDownloaded(bool error);
    void whazzupDownloading(int prog, int tot);
    void bookingsDownloading(int prog, int tot);
    void whazzupDownloaded(bool error);
    void bookingsDownloaded(bool error);

private:
    Whazzup();
    virtual ~Whazzup();

    WhazzupData data, predictedData;

    QDateTime predictedTime;

    QHttp *statusDownloader, *whazzupDownloader, *bookingsDownloader;
    QBuffer *statusBuffer, *whazzupBuffer, *bookingsBuffer;

    QStringList urls, gzurls;
    QString metarUrl, tafUrl, shorttafUrl, userLink, atisLink, message;

    QTime lastDownloadTime;

    QTimer *downloadTimer, *bookingsTimer;
};

#endif /*WHAZZUP_H_*/
