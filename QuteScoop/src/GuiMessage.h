/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef GUIMESSAGE_H
#define GUIMESSAGE_H

#include <QString>
#include <QDateTime>
#include <QTimer>
//#include "Window.h"

class GuiMessage : QObject {
    Q_OBJECT
public:
    enum GuiMessageType {
        Remove,         // remove all messages/progressbar with 'id'
        Splash,         // show message as splash screen
        ProgressBar,    // call once with at least 'msg', 'id', 'progress' and 'total',
                        // subsequently 'id' and progress is fine
        Temporary,      // remains visible some seconds
        InformationUserAttention, // important user information not related to errors / 'id' may be used as MessageBox->title
        Persistent,     // shown until "Remove" with same id is called
        Warning,        // something is not going smooth, shown longer than Temporary / 'id' may be used as MessageBox->title
        ErrorUserAttention, // some error, user attention required / 'id' may be used as MessageBox->title
        CriticalUserInteraction, // error that could potentially lead to instability / 'id' may be used as MessageBox->title
        FatalUserInteraction, // program needs to close / 'id' may be used as MessageBox->title
        _Update         // internally called to update
    };

    GuiMessage(QObject *parent, QString msg, GuiMessageType msgType, QString id, int progress, int total, int showMs):
            QObject(parent),
            _msg(msg),
            _id(id),
            _msgType(msgType),
            _progress(progress),
            _total(total),
            _showMs(showMs)
    {
        if (msgType == Temporary or msgType == Warning) {
            QTimer::singleShot(showMs, this, SLOT(remove()));
        }
    }

public slots:
    void remove() {
 //       dynamic_cast<Window::Window*> (this->parent())->showGuiMessage(_msg, GuiMessage::Remove, _id);
    }

private:
    QString _msg, _id;
    GuiMessageType _msgType;
    int _progress, _total;
    //QDateTime _showUntil;
    int _showMs;
};

#endif // GUIMESSAGE_H
