/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef BOOKEDATCDIALOG_H
#define BOOKEDATCDIALOG_H

#include <QSortFilterProxyModel>

#include "BookedAtcDialogModel.h"
#include "BookedAtcSortFilter.h"
#include "ui_BookedAtcDialog.h"

class BookedAtcDialog : public QDialog, private Ui::BookedAtcDialog {
    Q_OBJECT
public:
    static BookedAtcDialog *getInstance(bool createIfNoInstance = true);
    void destroyInstance();
    void refresh();

signals:
    void needBookings();

private slots:
    void newFilter();
    void modelSelected(const QModelIndex& index);
    void on_tbPredict_clicked();
    void on_dateFilter_dateChanged(QDate date);
    void on_timeFilter_timeChanged(QTime date);
    void on_spinHours_valueChanged(int val);
    void on_editFilter_textChanged(QString str);

private:
    BookedAtcDialog();

    BookedAtcDialogModel bookedAtcModel;
    BookedAtcSortFilter *bookedAtcSortModel;

    QTime timeFilter_old;
    QDate dateFilter_old;
};

#endif // BOOKEDATCDIALOG_H
