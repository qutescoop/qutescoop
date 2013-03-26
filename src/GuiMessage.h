/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef GUIMESSAGE_H
#define GUIMESSAGE_H

#include "_pch.h"

/** The GUI message system.
  Background idea is to provide a simple API to any class that wants to show
  messages, statii and progresses to the user.
**/

class GuiMessages : public QObject {
        Q_OBJECT
    public:
        static GuiMessages *instance(bool createIfNoInstance = true);

        ///////////////////////////////////////////////////////////////////////////
        // STATIC METHODS TO SEND MESSAGES
        /** temporary message: **/
        static void message(const QString &msg, const QString &id = "");
        /** also temporary, but with higher priority **/
        static void warning(const QString &msg, const QString &id = "");
        /** shown if no other messages available **/
        static void status (const QString &msg, const QString &id = "");
        /** just draw attention, no disturbance of the program flow **/
        static void infoUserAttention(const QString &msg, const QString &id = "");
        /** informational, user-confirmation required **/
        static void infoUserInteraction(const QString &msg, const QString &titleAndId = "");
        /** just draw attention, no disturbance of the program flow **/
        static void errorUserAttention      (const QString &msg, const QString &id = "");
        /** user-confirmation required **/
        static void criticalUserInteraction (const QString &msg, const QString &titleAndId);
        /** program needs to close **/
        static void fatalUserInteraction    (const QString &msg, const QString &titleAndId);
        /** set progress message (highly recommended, shown in the label) **/
        static void progress(const QString &id, const QString &msg);
        /** update progress value **/
        static void progress(const QString &id, int value, int maximum = -1);
        /** remove message **/
        static void remove(const QString &id);

        ///////////////////////////////////////////////////////////////////////////
        // METHODS TO SET ACTIVE OUTPUT WIDGETS
        void addStatusLabel(QLabel *label, bool hideIfNothingToDisplay = true);
        void removeStatusLabel(QLabel *label);

        void addProgressBar(QProgressBar *progressBar, bool hideIfNothingToDisplay = true);
        void removeProgressBar(QProgressBar *progressBar);

        ///////////////////////////////////////////////////////////////////////////
        // INTERNALLY USED CLASS
        class GuiMessage {
            public:
                enum Type { // type corresponding to priority
                    All = 101, Uninitialized = 100, Persistent = 10, ProgressBar = 7,
                    Temporary = 9, InformationUserInteraction = 6,
                    InformationUserAttention = 5, Warning = 4, ErrorUserAttention = 3,
                    CriticalUserInteraction = 1, FatalUserInteraction = 0
                };
                GuiMessage() : // needed for _METATYPE
                        id(""), msg(""), type(Uninitialized), progressValue(-1),
                        progressMaximum(-1), showMs(-1), shownSince(QDateTime()) {
                }
                GuiMessage(const QString &id, const Type &type, const QString &msg,
                           int progressValue = -1, int progressMaximum = -1) :
                        id(id), msg(msg), type(type), progressValue(progressValue),
                        progressMaximum(progressMaximum), showMs(-1), shownSince(QDateTime()) {
                }

                bool operator==(const GuiMessage *gm) const {
                    return id == gm->id && msg == gm->msg && type == gm->type &&
                            progressValue == gm->progressValue && progressMaximum == gm->progressMaximum &&
                            showMs == gm->showMs && shownSince == gm->shownSince;
                }
                ~GuiMessage() {} // needed for _METATYPE

                QString id, msg;
                Type type;
                int progressValue, progressMaximum, showMs;
                QDateTime shownSince;
        };
        ///////////////////////////////////////////////////////////////////////////
        // INTERNALLY USED METHODS (public to be callable out of static methods)
        void updateMessage(GuiMessage *gm, bool callUpdate = true);
        void removeMessageById(const QString &id, bool callUpdate = true);
    public slots:
        void labelDestroyed(QObject *obj);
        void progressBarDestroyed(QObject *obj);
    private slots:
        void update();
    private:
        GuiMessages();
        ~GuiMessages() { delete _currentStatusMessage; delete _currentProgressMessage; }

        void setStatusMessage(GuiMessage *gm, bool bold = false, bool italic = false,
                              bool instantRepaint = true);
        void setProgress(GuiMessage *gm, bool instantRepaint = true);

        GuiMessage *messageById(const QString &id, const GuiMessage::Type &type = GuiMessage::All);

        QHash<QLabel*, bool> _labels; // bool indicating hideIfNothingToDisplay
        QHash<QProgressBar*, bool> _bars; // bool indicating hideIfNothingToDisplay
        GuiMessage *_currentStatusMessage, *_currentProgressMessage;
        QMultiMap<int, GuiMessage*> _messages; // messages sorted by priority (= int of enum GuiMessage::Type)
        QTimer _timer;
};

Q_DECLARE_METATYPE(GuiMessages::GuiMessage) // needed for QVariant and QDebug compatibility
QDebug operator<< (QDebug dbg, const GuiMessages::GuiMessage *gm);

#endif // GUIMESSAGE_H
