/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef WHAZZUP_H_
#define WHAZZUP_H_

#include "_pch.h"

#include "WhazzupData.h"
#include "Window.h"
#include "GuiMessage.h"
#include "NetworkManager.h"

class Whazzup: public QObject {
        Q_OBJECT

    public:
        static Whazzup* getInstance();

        const WhazzupData& whazzupData() {
            return (predictedTime.isValid()?
                        predictedData: data);
        } // we fake it when predicting a certain time
        const WhazzupData& realWhazzupData() {
            return data;
        } // this is always the really downloaded thing

        void setPredictedTime(QDateTime predictedTime);
        QDateTime getPredictedTime() const {
            return predictedTime;
        }

        QString getUserLink(const QString& id) const,
        getAtisLink(const QString& id) const;

        QList <QPair <QDateTime, QString> > getDownloadedWhazzups() const;

    signals:
        void newData(bool isNew);
        void needBookings();

    public slots:
        void download();
        void fromFile(QString filename);
        void setStatusLocation(const QString& url);
        void downloadBookings();

    private slots:
        void processStatus(QNetworkReply* reply);
        void whazzupProgress(qint64 prog,qint64 tot);
        void processWhazzup(QNetworkReply* reply);
        void bookingsProgress(qint64 prog,qint64 tot);
        void processBookings(QNetworkReply* reply);

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
