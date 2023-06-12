#ifndef BOOKEDATCDIALOG_H_
#define BOOKEDATCDIALOG_H_

#include "ui_BookedAtcDialog.h"

#include "../models/BookedAtcDialogModel.h"
#include "../models/filters/BookedAtcSortFilter.h"

class BookedAtcDialog
    : public QDialog, private Ui::BookedAtcDialog {
    Q_OBJECT
    public:
        static BookedAtcDialog* instance(bool createIfNoInstance = true, QWidget* parent = 0);
        void destroyInstance();
        void refresh();
        void setDateTime(const QDateTime &dateTime);

    signals:
        void needBookings();

    protected:
        void closeEvent(QCloseEvent* event);

    private slots:
        void on_dateTimeFilter_dateTimeChanged(QDateTime date);
        void performSearch();
        void on_tbPredict_clicked();
        void on_spinHours_valueChanged(int val);
        void on_editFilter_textChanged(QString str);

    private:
        BookedAtcDialog(QWidget* parent);

        BookedAtcDialogModel* _bookedAtcModel;
        BookedAtcSortFilter* _bookedAtcSortModel;

        QDateTime _dateTimeFilter_old;
        QTimer _editFilterTimer;

        constexpr static char m_preferencesName[] = "bookAtcDialog";
};

#endif // BOOKEDATCDIALOG_H
