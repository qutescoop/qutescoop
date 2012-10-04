/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef WHAZZUP_H_
#define WHAZZUP_H_

#include "_pch.h"

#include "WhazzupData.h"
#include "Window.h"
#include "GuiMessage.h"
#include "networkmanager.h"

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
    void statusDownloaded(QNetworkReply* reply);
    void whazzupDownloaded(QNetworkReply* reply);
    void bookingsDownloaded(QNetworkReply* reply);

private:
    Whazzup();
    virtual ~Whazzup();

    WhazzupData data, predictedData;

    QDateTime predictedTime;

    QStringList urls, gzurls;
    QString metarUrl, tafUrl, shorttafUrl, userLink, atisLink, message;

    QTime lastDownloadTime;

    QTimer *downloadTimer, *bookingsTimer;
};

#endif /*WHAZZUP_H_*/
