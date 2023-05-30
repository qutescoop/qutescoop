/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "GuiMessage.h"

#include <QApplication>
#include <QMessageBox>

GuiMessages *guiMessagesInstance = 0;
GuiMessages *GuiMessages::instance(bool createIfNoInstance) {
    if(guiMessagesInstance == 0)
        if (createIfNoInstance)
            guiMessagesInstance = new GuiMessages();
    return guiMessagesInstance;
}
GuiMessages::GuiMessages() :
        _currentStatusMessage(new GuiMessage()),
        _currentProgressMessage(new GuiMessage()) {
    _timer.setSingleShot(true);
    connect(&_timer, &QTimer::timeout, this, &GuiMessages::update);
}

///////////////////////////////////////////////////////////////////////////
// STATIC METHODS TO SEND MESSAGES
// text:
void GuiMessages::message(const QString &msg, const QString &id) {
    GuiMessages::instance()->updateMessage(new GuiMessage(id, GuiMessage::Temporary, msg));
}
void GuiMessages::warning(const QString &msg, const QString &id) {
    GuiMessages::instance()->updateMessage(new GuiMessage(id, GuiMessage::Warning, msg));
}
void GuiMessages::status (const QString &msg, const QString &id) {
    GuiMessages::instance()->updateMessage(new GuiMessage(id, GuiMessage::Persistent, msg));
}
void GuiMessages::infoUserAttention(const QString &msg, const QString &id) {
    GuiMessages::instance()->updateMessage(
                new GuiMessage(id, GuiMessage::InformationUserAttention, msg));
}
void GuiMessages::infoUserInteraction(const QString &msg, const QString &titleAndId) {
    GuiMessages::instance()->updateMessage(
                new GuiMessage(titleAndId, GuiMessage::InformationUserInteraction, msg));
}
void GuiMessages::errorUserAttention      (const QString &msg, const QString &id) {
    GuiMessages::instance()->updateMessage(
                new GuiMessage(id, GuiMessage::ErrorUserAttention, msg));
}
void GuiMessages::criticalUserInteraction (const QString &msg, const QString &titleAndId) {
    GuiMessages::instance()->updateMessage(
                new GuiMessage(titleAndId, GuiMessage::CriticalUserInteraction, msg));
}
void GuiMessages::fatalUserInteraction    (const QString &msg, const QString &titleAndId) {
    GuiMessages::instance()->updateMessage(
                new GuiMessage(titleAndId, GuiMessage::FatalUserInteraction, msg));
}
// progress (including text):
void GuiMessages::progress(const QString &id, int value, int maximum) {
    GuiMessages::instance()->updateMessage(
                new GuiMessage(id, GuiMessage::ProgressBar, "", value, maximum));
}
void GuiMessages::progress(const QString &id, const QString &msg) {
    GuiMessages::instance()->updateMessage(
                new GuiMessage(id, GuiMessage::ProgressBar, msg));
}

void GuiMessages::remove(const QString &id) {
    if (!id.isEmpty())
        GuiMessages::instance()->removeMessageById(id);
}

///////////////////////////////////////////////////////////////////////////
// METHODS TO SET ACTIVE OUTPUT WIDGETS
void GuiMessages::addStatusLabel(QLabel *label, bool hideIfNothingToDisplay) {
    //qDebug() << "GuiMessages::addStatusLabel()" << label->objectName();
    // we want to be notified before this QLabel is getting invalid
    connect(label, &QObject::destroyed, this, &GuiMessages::labelDestroyed);
    _labels.insert(label, hideIfNothingToDisplay);
    update();
}
void GuiMessages::removeStatusLabel(QLabel *label) {
    //qDebug() << "GuiMessages::removeStatusLabel()" << label->objectName();
    if(_labels[label])
        label->hide();
    _labels.remove(label);
}
void GuiMessages::labelDestroyed(QObject *obj) {
    if (static_cast<QLabel*>(obj) != 0) {
        if (_labels.remove(static_cast<QLabel*>(obj)) == 0)
            qWarning() << "GuiMessages::labelDestroyed() object not found";
        // else
        //    qDebug() << "GuiMessages::labelDestroyed() removed object";
    } else
        qWarning() << "GuiMessages::labelDestroyed() invalid object cast";
}

void GuiMessages::addProgressBar(QProgressBar *progressBar, bool hideIfNothingToDisplay) {
    //qDebug() << "GuiMessages::addProgressBar()" << progressBar->objectName();
    // we want to be notified before this QProgressBar is getting invalid
    connect(progressBar, &QObject::destroyed, this, &GuiMessages::progressBarDestroyed);
    _bars.insert(progressBar, hideIfNothingToDisplay);
    update();
}
void GuiMessages::removeProgressBar(QProgressBar *progressBar) {
    //qDebug() << "GuiMessages::removeProgressBar()" << progressBar->objectName();
    if(_bars[progressBar])
        progressBar->hide();
    _bars.remove(progressBar);
}
void GuiMessages::progressBarDestroyed(QObject *obj) {
    // qDebug() << obj;
    if (static_cast<QProgressBar*>(obj) != 0) {
        if (_bars.remove(static_cast<QProgressBar*>(obj)) == 0)
            qWarning() << "GuiMessages::progressBarDestroyed() object not found";
        // else
        //    qDebug() << "GuiMessages::progressBarDestroyed() removed object";
    } else
        qWarning() << "GuiMessages::progressBarDestroyed() invalid object cast";
}

///////////////////////////////////////////////////////////////////////////
// INTERNALLY USED CLASS AND METHODS (called by static methods)
void GuiMessages::updateMessage(GuiMessage *gm) {
    //qDebug() << "GuiMessages::updateMessage()" << guiMessage;
    GuiMessage *existing = messageById(gm->id, gm->type);
    if (existing != 0) {
        if (!gm->msg.isEmpty())
            existing->msg             = gm->msg;
        if (gm->progressValue != -1)
            existing->progressValue   = gm->progressValue;
        if (gm->progressMaximum != -1)
            existing->progressMaximum = gm->progressMaximum;
        if (gm->showMs != -1)
            existing->showMs          = gm->showMs;
        if (gm->shownSince.isValid())
            existing->shownSince      = gm->shownSince;
        // qDebug() << " updated existing message:" << existing;
    } else {
        // qDebug() << "GuiMessage()::updateMessage() new" << gm;
        _messages.insert(gm->type, gm);
    }
    update();
}
void GuiMessages::removeMessageById(const QString &id) {
    // qDebug() << "GuiMessages::removeMessage() id=" << id;
    foreach(int key, _messages.keys()) {
        foreach(GuiMessage *gm, _messages.values(key)) {
            if (gm->id == id) {
                if (_currentStatusMessage == gm)
                    setStatusMessage(new GuiMessage());
                if (_currentProgressMessage == gm)
                    setProgress(new GuiMessage());
                // qDebug() << "GuiMessage()::removeMessageById() removed"
                //         << gm;
                _messages.remove(key, gm);
            }
        }
    }
    update();
}

///////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
void GuiMessages::setStatusMessage(GuiMessage *gm, bool bold, bool italic, bool instantRepaint) {
    Q_UNUSED(bold);
    Q_UNUSED(italic);
    //qDebug() << "GuiMessages::setStatusMessage()" << gm;
    foreach(QLabel *l, _labels.keys()) {
        l->setText(gm->msg);
        if (_labels[l]) { // bool indicating hideIfNothingToDisplay
            const bool visible = !gm->msg.isEmpty()
                    && (gm->type != GuiMessage::Uninitialized);
            l->setVisible(visible);
        }
        if (instantRepaint)
            l->repaint();
    }
    _currentStatusMessage = (gm->msg.isEmpty()? 0: gm);
    if (!gm->shownSince.isValid())
        gm->shownSince = QDateTime::currentDateTimeUtc();
}
void GuiMessages::setProgress(GuiMessage *gm, bool instantRepaint) {
    //qDebug() << "GuiMessages::setProgress()" << gm;
    foreach(QProgressBar *pb, _bars.keys()) {
        if (gm->progressMaximum != -1)
            pb->setMaximum(gm->progressMaximum);
        pb->setValue(gm->progressValue);
        if (_bars[pb]) { // boolean inicating hideIfNohingToDisplay
            const bool visible =
                    (gm->progressValue != gm->progressMaximum) ||
                    (gm->progressMaximum == -1 && gm->progressValue != -1);
            pb->setVisible(visible);
        }
        if (instantRepaint)
            pb->repaint();
    }
    _currentProgressMessage = gm;
}

void GuiMessages::update() {
    //qDebug() << "GuiMessages::update() now=" << QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    foreach(int key, _messages.keys()) {
        foreach(GuiMessage *gm, _messages.values(key)) {
            //qDebug() << " " << gm;

            if (gm->showMs != -1 && gm->shownSince.isValid() && // shown long enough
                gm->shownSince.addMSecs(gm->showMs)
                < QDateTime::currentDateTimeUtc()) {
                removeMessageById(gm->id);
            } else {
                switch (gm->type) {
                    case GuiMessage::FatalUserInteraction:
                        QMessageBox::critical(nullptr, gm->id, gm->msg);
                        qFatal("%s %s", (const char*) gm->id.constData(), (const char*) gm->msg.constData());
                        _messages.remove(key, gm);
                        return;
                    case GuiMessage::CriticalUserInteraction:
                        QMessageBox::critical(nullptr, gm->id, gm->msg);
                        qCritical("%s %s", (const char*) gm->id.constData(), (const char*) gm->msg.constData());
                        _messages.remove(key, gm);
                        return;
                    case GuiMessage::ErrorUserAttention:
                        gm->showMs = 5000;
                        setStatusMessage(gm, true);
                        _timer.start(gm->showMs);
                        return;
                    case GuiMessage::Warning:
                        gm->showMs = 3000;
                        setStatusMessage(gm, false, true);
                        _timer.start(gm->showMs);
                        return;
                    case GuiMessage::InformationUserAttention:
                        gm->showMs = 3000;
                        setStatusMessage(gm, true);
                        _timer.start(gm->showMs);
                        return;
                    case GuiMessage::InformationUserInteraction:
                        QMessageBox::information(nullptr, gm->id, gm->msg);
                        _messages.remove(key, gm);
                        return;
                    case GuiMessage::ProgressBar:
                        gm->showMs = 10000; // timeout value
                        setStatusMessage(gm);
                        _timer.start(gm->showMs);
                        if (gm->progressValue != -1)
                            setProgress(gm);
                        break; // looking further for messages
                    case GuiMessage::Temporary:
                        gm->showMs = 3000;
                        setStatusMessage(gm);
                        _timer.start(gm->showMs);
                        return;
                    case GuiMessage::Persistent:
                        setStatusMessage(gm);
                        setProgress(new GuiMessage());
                        return;
                    default: {}
                }
            }
        }
    }
    //qDebug() << "GuiMessages::update() --finished";
}

GuiMessages::GuiMessage *GuiMessages::messageById(const QString &id, const GuiMessage::Type &type) {
    foreach(GuiMessage *gm, _messages)
        if (gm->id == id && (type == GuiMessage::All || gm->type == type))
            return gm;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// GuiMessages::GuiMessage qDebug() compatibility
QDebug operator<<(QDebug dbg, const GuiMessages::GuiMessage *gm) {
    dbg.nospace() << "id: " << gm->id << ", msg: " << gm->msg
                  << ", type " << gm->type << ", val: " << gm->progressValue
                  << ", max: " << gm->progressMaximum
                  << ", since: " << gm->shownSince.toString(Qt::ISODate)
                  << ", ms: " << gm->showMs;
    return dbg.maybeSpace();
}
