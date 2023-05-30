#ifndef JOBLIST_H
#define JOBLIST_H

#include <QObject>

/**
  A list of jobs, connected through SIGNALs and SLOTs. Will emit finished()
  when all jobs were executed
**/
class JobList : public QObject {
        Q_OBJECT
    public:
        explicit JobList(QObject *parent = 0);
        class Job {
            public:
                Job(QObject *obj, const char *start, const char *finishSignal);
                QObject *obj;
                const char *start, *finishSignal;
        };
        void append(Job job);
    signals:
        void started();
        void finished();
    public slots:
        void start();
    private slots:
        void finish();
    private:
        QList<Job> jobs;
};

#endif // JOBLIST_H
