/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "GuiMessage.h"

////////////////////////////////////////////////////////////////////////
// GuiMessageProxy (singleton)
////////////////////////////////////////////////////////////////////////

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
    connect(&_timer, SIGNAL(timeout()), this, SLOT(update()));
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
    connect(label, SIGNAL(destroyed(QObject*)), this, SLOT(labelDestroyed(QObject*)));
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
    //qDebug() << "GuiMessages::labelDestroyed() received SIGNAL";
    _labels.remove(dynamic_cast<QLabel*>(obj));
}

void GuiMessages::addProgressBar(QProgressBar *progressBar, bool hideIfNothingToDisplay) {
    //qDebug() << "GuiMessages::addProgressBar()" << progressBar->objectName();
    // we want to be notified before this QProgressBar is getting invalid
    connect(progressBar, SIGNAL(destroyed(QObject*)), this, SLOT(progressBarDestroyed(QObject*)));
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
    //qDebug() << "GuiMessages::progressBarDestroyed() received SIGNAL";
    _bars.remove(dynamic_cast<QProgressBar*>(obj));
}

///////////////////////////////////////////////////////////////////////////
// INTERNALLY USED CLASS AND METHODS (called by static methods)
void GuiMessages::updateMessage(GuiMessage *guiMessage, bool callUpdate) {
    //qDebug() << "GuiMessages::updateMessage()" << guiMessage;
    GuiMessage *existing = messageById(guiMessage->id, guiMessage->type);
    if (existing != 0) {
        if (!guiMessage->msg.isEmpty())
            existing->msg             = guiMessage->msg;
        if (guiMessage->progressValue != -1)
            existing->progressValue   = guiMessage->progressValue;
        if (guiMessage->progressMaximum != -1)
            existing->progressMaximum = guiMessage->progressMaximum;
        if (guiMessage->showMs != -1)
            existing->showMs          = guiMessage->showMs;
        if (guiMessage->shownSince.isValid())
            existing->shownSince      = guiMessage->shownSince;
        //qDebug() << " updated existing message:" << existing;
    } else
        _messages.insert(guiMessage->type, guiMessage);
    if (callUpdate)
        update();
}
void GuiMessages::removeMessageById(const QString &id, bool callUpdate) {
    //qDebug() << "GuiMessages::removeMessage() id=" << id;
    foreach(int key, _messages.keys()) {
        foreach(GuiMessage *gm, _messages.values(key)) {
            if (gm->id == id) {
                if (_currentStatusMessage == gm)
                    setStatusMessage(new GuiMessage());
                if (_currentProgressMessage == gm)
                    setProgress(new GuiMessage());
                _messages.remove(key, gm);
            }
        }
    }
    if (callUpdate)
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
                        QMessageBox::critical(qApp->desktop(), gm->id, gm->msg);
                        qFatal("%s %s", (const char*) gm->id.constData(), (const char*) gm->msg.constData());
                        _messages.remove(key, gm);
                        return;
                    case GuiMessage::CriticalUserInteraction:
                        QMessageBox::critical(qApp->desktop(), gm->id, gm->msg);
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
                        QMessageBox::information(qApp->desktop(), gm->id, gm->msg);
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
// GuiMessages::GuiMessage QDebug compatibility
QDebug operator<<(QDebug dbg, const GuiMessages::GuiMessage *gm) {
    dbg.nospace() << "id: " << gm->id << ", msg: " << gm->msg
                  << ", type " << gm->type << ", val: " << gm->progressValue
                  << ", max: " << gm->progressMaximum
                  << ", since: " << gm->shownSince.toString(Qt::ISODate)
                  << ", ms: " << gm->showMs;
    return dbg.maybeSpace();
}
