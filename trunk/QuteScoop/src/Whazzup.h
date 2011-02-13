/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef WHAZZUP_H_
#define WHAZZUP_H_

#include "_pch.h"

#include "WhazzupData.h"
#include "Window.h"
#include "GuiMessage.h"

class WhazzupData;

class Whazzup: public QObject
{
    Q_OBJECT

public:
    static Whazzup* getInstance();

    /**
     * Set the download location for whazzup status file
     */
    const WhazzupData& whazzupData() { return (predictedTime.isValid()? predictedData: data); } // we fake it when predicting a certain time
    const WhazzupData& realWhazzupData() { return data; } // this is always the really downloaded thing

    void setPredictedTime(QDateTime predictedTime);
    QDateTime getPredictedTime() const { return predictedTime; }

    QString getUserLink(const QString& id) const;
    QString getAtisLink(const QString& id) const;

    QList <QPair <QDateTime, QString> > getDownloadedWhazzups();

signals:
    void newData(bool isNew);
    void hasGuiMessage(QString, GuiMessage::GuiMessageType = GuiMessage::Temporary,
                       QString = QString(), int = 0, int = 0);
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

    WhazzupData data;
    WhazzupData predictedData;

    QDateTime predictedTime;

    QHttp *statusDownloader;
    QBuffer *statusBuffer;

    QHttp *whazzupDownloader;
    QBuffer *whazzupBuffer;

    QHttp *bookingsDownloader;
    QBuffer *bookingsBuffer;

    QStringList urls;
    QStringList gzurls;
    QString metarUrl;
    QString tafUrl;
    QString shorttafUrl;
    QString userLink;
    QString atisLink;
    QString message;

    QTime lastDownloadTime;

    QTimer *downloadTimer, *bookingsTimer;
};

#endif /*WHAZZUP_H_*/
