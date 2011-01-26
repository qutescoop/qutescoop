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
    static BookedAtcDialog *getInstance(bool createIfNoInstance = true, QWidget *parent = 0);
    void destroyInstance();
    void refresh();

signals:
    void needBookings();

private slots:
    void performSearch();
    void modelSelected(const QModelIndex& index);
    void on_tbPredict_clicked();
    void on_dateFilter_dateChanged(QDate date);
    void on_timeFilter_timeChanged(QTime date);
    void on_spinHours_valueChanged(int val);
    void on_editFilter_textChanged(QString str);

private:
    BookedAtcDialog(QWidget *parent);

    BookedAtcDialogModel bookedAtcModel;
    BookedAtcSortFilter *bookedAtcSortModel;

    QTime timeFilter_old;
    QDate dateFilter_old;

    QTimer searchTimer;
};

#endif // BOOKEDATCDIALOG_H