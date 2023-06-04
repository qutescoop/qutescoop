#include "GuiMessage.h"
#include "JobList.h"

JobList::JobList(QObject* parent)
    : QObject(parent) {}

void JobList::append(JobList::Job job) {
    m_jobs.append(job);
}

void JobList::start() {
    for (int i = 0; i < m_jobs.size(); i++) {
        connect(
            m_jobs[i].obj,
            m_jobs[i].finishSignal,
            this,
            SLOT(advanceProgress())
        );

        if (i + 1 < m_jobs.size()) {
            m_jobs[i + 1].obj->connect(
                m_jobs[i].obj,
                m_jobs[i].finishSignal,
                m_jobs[i + 1].start
            );
        } else {
            connect(
                m_jobs[i].obj,
                m_jobs[i].finishSignal,
                this,
                SLOT(finish())
            );
        }
    }
    if (m_jobs.isEmpty()) {
        connect(this, &JobList::started, this, &JobList::finish);
    } else {
        m_jobs[0].obj->connect(this, SIGNAL(started()), m_jobs[0].start);
    }

    GuiMessages::progress("joblist", m_progress, 100);
    emit started();
}

void JobList::advanceProgress() {
    if (m_jobs.isEmpty()) {
        return;
    }
    m_progress += 100. / m_jobs.size();
    GuiMessages::progress("joblist", m_progress, 100);
}

void JobList::finish() {
    emit finished();
}

JobList::Job::Job(QObject* obj, const char* start, const char* finishSignal)
    : obj(obj), start(start), finishSignal(finishSignal) {}
