/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include <QHeaderView>

#include "BookedAtcDialog.h"
#include "BookedAtcDialogModel.h"
#include "BookedAtcSortFilter.h"
#include "Whazzup.h"
#include "Window.h"
#include "Settings.h"
#include "ui_MainWindow.h"

// singleton instance
BookedAtcDialog *bookedAtcDialog = 0;
BookedAtcDialog *BookedAtcDialog::getInstance(bool createIfNoInstance, QWidget *parent) {
    if(bookedAtcDialog == 0)
        if (createIfNoInstance) {
            if (parent != 0) bookedAtcDialog = new BookedAtcDialog(parent);
        }
    return bookedAtcDialog;
}

// destroys a singleton instance
void BookedAtcDialog::destroyInstance() {
    delete bookedAtcDialog;
    bookedAtcDialog = 0;
}


BookedAtcDialog::BookedAtcDialog(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
//    setWindowFlags(Qt::Tool);

    //bookedAtcSortModel = new QSortFilterProxyModel;
    qDebug() << "BookedAtcDialog(): bookedAtcSortModel1";
    bookedAtcSortModel = new BookedAtcSortFilter;

    // slows down considerably
    bookedAtcSortModel->setDynamicSortFilter(true);
    bookedAtcSortModel->setSourceModel(&bookedAtcModel);
    qDebug() << "BookedAtcDialog(): treeBookedAtc1";
    treeBookedAtc->setModel(bookedAtcSortModel);
    treeBookedAtc->header()->setResizeMode(QHeaderView::Interactive);
    treeBookedAtc->sortByColumn(4, Qt::AscendingOrder);

    connect(treeBookedAtc->header(), SIGNAL(sectionClicked(int)), treeBookedAtc, SLOT(sortByColumn(int)));
    connect(treeBookedAtc, SIGNAL(clicked(const QModelIndex&)), this, SLOT(modelSelected(const QModelIndex&)));
    //connect(bookedAtcSortModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(newFilter(QModelIndex,QModelIndex))); //does not get thrown??

    QFont font = lblStatusInfo->font();
    font.setPointSize(lblStatusInfo->fontInfo().pointSize() - 1);
    lblStatusInfo->setFont(font); //make it a bit smaller than standard text

    connect(this, SIGNAL(needBookings()), Whazzup::getInstance(), SLOT(downloadBookings()));

    connect(&searchTimer, SIGNAL(timeout()), this, SLOT(performSearch()));

    qDebug() << "BookedAtcDialog(): calling refresh";
    dateFilter->setDate(QDateTime::currentDateTime().toUTC().date());
    timeFilter->setTime(QDateTime::currentDateTime().toUTC().time());
    refresh();
}

void BookedAtcDialog::refresh() {
    //bookedAtcSortModel->setDynamicSortFilter(false);

    if(Settings::downloadBookings() &&
       !Whazzup::getInstance()->realWhazzupData().bookingsTimestamp().isValid())
        emit needBookings();

    qDebug() << "BookedAtcDialog/refresh(): setting clients";
    bookedAtcModel.setClients(Whazzup::getInstance()->realWhazzupData().getBookedControllers());
    qDebug() << "BookedAtcDialog/refresh(): resizing headers";
    bookedAtcSortModel->invalidate();
    treeBookedAtc->header()->resizeSections(QHeaderView::ResizeToContents);

    qDebug() << "BookedAtcDialog/refresh(): getting Whazzup";
    const WhazzupData &data = Whazzup::getInstance()->realWhazzupData();
    qDebug() << "BookedAtcDialog/refresh(): ready";

    QString msg = QString("Bookings %1 updated")
                  .arg(data.bookingsTimestamp().date() == QDateTime::currentDateTime().toUTC().date() // is today?
                        ? QString("today %1").arg(data.bookingsTimestamp().time().toString("HHmm'z'"))
                        : (data.bookingsTimestamp().isValid()
                           ? data.bookingsTimestamp().toString("ddd MM/dd HHmm'z'")
                           : "never")
                        );
    lblStatusInfo->setText(msg);

//    bookedAtcSortModel->setDynamicSortFilter(true);

    searchTimer.start(5);
}

void BookedAtcDialog::on_editFilter_textChanged(QString searchStr)
{
    searchTimer.start(1000);
}

void BookedAtcDialog::on_spinHours_valueChanged(int val)
{
    searchTimer.start(1000);
}


void BookedAtcDialog::on_timeFilter_timeChanged(QTime time)
{
    QTime newTime;
    // make hour change if 59+ or 0-
    if (timeFilter_old.minute() == 59 && time.minute() == 0)
        newTime = time.addSecs(60 * 60);
    if (timeFilter_old.minute() == 0 && time.minute() == 59)
        newTime = time.addSecs(-60 * 60);

    // make date change if 23+ or 00-
    if (timeFilter_old.hour() == 23 && time.hour() == 0)
        dateFilter->setDate(dateFilter->date().addDays(1));
    if (timeFilter_old.hour() == 0 && time.hour() == 23)
        dateFilter->setDate(dateFilter->date().addDays(-1));

    timeFilter_old = time;
    if (newTime.isValid()) {
        timeFilter->setTime(newTime);
        return;
    }

    searchTimer.start(1000);
}

void BookedAtcDialog::on_dateFilter_dateChanged(QDate date)
{
    QDate newDate;
    // make month change if lastday+ or 0-
    if (dateFilter_old.day() == dateFilter_old.daysInMonth() && date.day() == 1)
        newDate = date.addMonths(1);
    if (dateFilter_old.day() == 1 && date.day() == date.daysInMonth())
        newDate = date.addMonths(-1);

    dateFilter_old = date;
    if(newDate.isValid()) {
        dateFilter->setDate(newDate);
        return;
    }

    searchTimer.start(1000);
}

void BookedAtcDialog::performSearch() {
    searchTimer.stop();
    //bookedAtcSortModel->setDynamicSortFilter(false);

    qDebug() << "BookedAtcDialog/performSearch(): building RegExp";
    // Text
    QRegExp regex;
    QStringList tokens = editFilter->text().trimmed().replace(QRegExp("\\*"), ".*").split(QRegExp("[ \\,]+"), QString::SkipEmptyParts);
    if(tokens.size() == 1) {
        regex = QRegExp("^" + tokens.first() + ".*", Qt::CaseInsensitive);
    } else if(tokens.size() == 0) {
        regex = QRegExp("");
    }
    else {
        QString regExpStr = "^(" + tokens.first();
        for(int i = 1; i < tokens.size(); i++)
            regExpStr += "|" + tokens[i];
        regExpStr += ".*)";
        regex = QRegExp(regExpStr, Qt::CaseInsensitive);
    }

    qDebug() << "BookedAtcDialog/performSearch(): setting RegExp" << regex.pattern();
    bookedAtcSortModel->setFilterKeyColumn(-1);
    bookedAtcSortModel->setFilterRegExp(regex);

    //Date, Time, TimeSpan
    QDateTime from = QDateTime(dateFilter->date(), timeFilter->time(), Qt::UTC);
    QDateTime to = from.addSecs(spinHours->value() * 3600);
    qDebug() << "BookedAtcDialog/performSearch(): setting Date" << from << to;
    bookedAtcSortModel->setDateTimeRange(from, to);


    qDebug() << "BookedAtcDialog/performSearch(): applying filter";
    //bookedAtcSortModel->setDynamicSortFilter(true);
    // General
    qDebug() << "BookedAtcDialog/performSearch(): resizing headers";
    treeBookedAtc->header()->resizeSections(QHeaderView::ResizeToContents);
    qDebug() << "BookedAtcDialog/performSearch(): rowCount()";
    boxResults->setTitle(QString("Results (%1)").arg(bookedAtcSortModel->rowCount()));
    qDebug() << "BookedAtcDialog/performSearch() -- finished";
}

void BookedAtcDialog::modelSelected(const QModelIndex& index) {
    bookedAtcModel.modelSelected(bookedAtcSortModel->mapToSource(index));
}

void BookedAtcDialog::on_tbPredict_clicked()
{
    hide();
    Whazzup::getInstance()->setPredictedTime(QDateTime(dateFilter->date(), timeFilter->time(), Qt::UTC));
}
