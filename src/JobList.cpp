/**************************************************************************
 This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "JobList.h"

JobList::JobList(QObject *parent) :
    QObject(parent) {
}

void JobList::append(JobList::Job job) {
    jobs.append(job);
}

void JobList::start() {
    // qDebug() << "JobList::start()";
    for (int i = 0; i < jobs.size(); i++) {
        if (i + 1 < jobs.size()) {
            // qDebug() << "JobList::start() connecting" << jobs[i].obj << jobs[i].finishSignal
            //         << ">>" << jobs[i + 1].obj << jobs[i + 1].start;
            jobs[i + 1].obj->connect(
                    jobs[i].obj, jobs[i].finishSignal,
                    jobs[i + 1].start
            );
        } else {
            // qDebug() << "JobList::start() connecting" << jobs[i].obj << jobs[i].finishSignal
            //         << ">>" << this << SLOT(finish());
            connect(jobs[i].obj, jobs[i].finishSignal,
                    SLOT(finish()));
        }
    }
    if (jobs.isEmpty()) {
        connect(this, &JobList::started, this, &JobList::finish);
    }
    else {
        jobs[0].obj->connect(this, SIGNAL(started()), jobs[0].start);
        //jobs[0].obj->metaObject()->invokeMethod(jobs[0].obj, jobs[0].start);
    }
    // qDebug() << "JobList::start() -- finished. Jobs are started.";
    emit started();
}

void JobList::finish() {
    // qDebug() << "JobList::finish() emitting finished()";
    emit finished();
}

JobList::Job::Job(QObject *obj, const char *start, const char *finishSignal) :
    obj(obj), start(start), finishSignal(finishSignal) {
}
