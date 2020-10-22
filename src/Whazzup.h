/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef WHAZZUP_H_
#define WHAZZUP_H_

#include "_pch.h"

#include "WhazzupData.h"
#include "Window.h"
#include "GuiMessage.h"
#include "Net.h"

class Whazzup: public QObject {
        Q_OBJECT
    public:
        static Whazzup* instance();

        const WhazzupData& whazzupData() {
            return (predictedTime.isValid()?
                        _predictedData: _data);
        } // we fake it when predicting a certain time
        const WhazzupData& realWhazzupData() {
            return _data;
        } // this is always the really downloaded thing

        void setPredictedTime(QDateTime predictedTime);
        QString userLink(const QString& id) const,
                atisLink(const QString& id) const;
        QList <QPair <QDateTime, QString> > downloadedWhazzups() const;
        QDateTime predictedTime;
    signals:
        void newData(bool isNew);
        void whazzupDownloaded();
        void needBookings();
    public slots:
        void download();
        void fromFile(QString filename);
        void setStatusLocation(const QString& url);
        void downloadBookings();
    private slots:
        void processStatus();
        void whazzupProgress(qint64 prog,qint64 tot);
        void processWhazzup();
        void bookingsProgress(qint64 prog,qint64 tot);
        void processBookings();
    private:
        Whazzup();
        virtual ~Whazzup();

        WhazzupData _data, _predictedData;
        QStringList _urls, _gzurls;
        QString _metarUrl, _tafUrl, _shorttafUrl, _userLink, _atisLink, _message;
        QTime _lastDownloadTime;
        QTimer *_downloadTimer, *_bookingsTimer;
        QNetworkReply *_replyStatus, *_replyWhazzup, *_replyBookings;
};

#endif /*WHAZZUP_H_*/
