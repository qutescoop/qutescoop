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
    if(bookedAtcDialog == 0) {
        if (createIfNoInstance) {
            if (parent == 0) parent = Window::getInstance(true);
            bookedAtcDialog = new BookedAtcDialog(parent);
        }
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
    setWindowFlags(windowFlags() ^= Qt::WindowContextHelpButtonHint);
//    setWindowFlags(Qt::Tool);
    bookedAtcSortModel = new BookedAtcSortFilter;

    bookedAtcSortModel->setDynamicSortFilter(true);
    bookedAtcSortModel->setSourceModel(&bookedAtcModel);

    treeBookedAtc->setUniformRowHeights(true);
    treeBookedAtc->setModel(bookedAtcSortModel);

    treeBookedAtc->header()->setResizeMode(QHeaderView::Interactive);
    treeBookedAtc->sortByColumn(4, Qt::AscendingOrder);

    connect(treeBookedAtc->header(), SIGNAL(sectionClicked(int)), treeBookedAtc, SLOT(sortByColumn(int)));
    connect(treeBookedAtc, SIGNAL(clicked(const QModelIndex&)), this, SLOT(modelSelected(const QModelIndex&)));
    //connect(bookedAtcSortModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(newFilter(QModelIndex,QModelIndex))); //does not get thrown??

    QFont font = lblStatusInfo->font();
    font.setPointSize(lblStatusInfo->fontInfo().pointSize() - 1);
    lblStatusInfo->setFont(font); //make it a bit smaller than standard text

    dateTimeFilter->setDateTime(QDateTime::currentDateTime().toUTC());

    connect(&editFilterTimer, SIGNAL(timeout()), this, SLOT(performSearch()));
    performSearch();
    connect(this, SIGNAL(needBookings()), Whazzup::getInstance(), SLOT(downloadBookings()));
    refresh();
}

void BookedAtcDialog::refresh() {
    if(Settings::downloadBookings() &&
       !Whazzup::getInstance()->realWhazzupData().bookingsTimestamp().isValid())
        emit needBookings();

    const WhazzupData &data = Whazzup::getInstance()->realWhazzupData();

    qDebug() << "BookedAtcDialog/refresh(): setting clients";
    bookedAtcModel.setClients(data.getBookedControllers());

    QString msg = QString("Bookings %1 updated")
                  .arg(data.bookingsTimestamp().date() == QDateTime::currentDateTime().toUTC().date() // is today?
                        ? QString("today %1").arg(data.bookingsTimestamp().time().toString("HHmm'z'"))
                        : (data.bookingsTimestamp().isValid()
                           ? data.bookingsTimestamp().toString("ddd MM/dd HHmm'z'")
                           : "never")
                        );
    lblStatusInfo->setText(msg);

    qDebug() << "BookedAtcDialog/refresh() -- finished";

    editFilterTimer.start(5);
}

void BookedAtcDialog::on_dateTimeFilter_dateTimeChanged(QDateTime dateTime)
{
    // some niceify on the default behaviour, making the sections depend on each other
    disconnect(dateTimeFilter, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(on_dateTimeFilter_dateTimeChanged(QDateTime)));
    editFilterTimer.stop();

    // make year change if M 12+ or 0-
    if ((dateTimeFilter_old.date().month() == 12)
        && (dateTime.date().month() == 1))
        dateTime = dateTime.addYears(1);
    if ((dateTimeFilter_old.date().month() == 1)
        && (dateTime.date().month() == 12))
        dateTime = dateTime.addYears(-1);

    // make month change if d lastday+ or 0-
    if ((dateTimeFilter_old.date().day() == dateTimeFilter_old.date().daysInMonth())
        && (dateTime.date().day() == 1)) {
        dateTime = dateTime.addMonths(1);
    }
    if ((dateTimeFilter_old.date().day() == 1)
        && (dateTime.date().day() == dateTime.date().daysInMonth())) {
        dateTime = dateTime.addMonths(-1);
        dateTime = dateTime.addDays( // compensate for month lengths
                dateTime.date().daysInMonth()
                - dateTimeFilter_old.date().daysInMonth());
    }

    // make day change if h 23+ or 00-
    if ((dateTimeFilter_old.time().hour() == 23)
        && (dateTime.time().hour() == 0))
        dateTime = dateTime.addDays(1);
    if ((dateTimeFilter_old.time().hour() == 0)
        && (dateTime.time().hour() == 23))
        dateTime = dateTime.addDays(-1);

    // make hour change if m 59+ or 0-
    if ((dateTimeFilter_old.time().minute() == 59)
        && (dateTime.time().minute() == 0))
        dateTime = dateTime.addSecs(60 * 60);
    if ((dateTimeFilter_old.time().minute() == 0)
        && (dateTime.time().minute() == 59))
        dateTime = dateTime.addSecs(-60 * 60);

    dateTimeFilter_old = dateTime;

    if(dateTime.isValid()
        && (dateTime != dateTimeFilter->dateTime())) {
        dateTimeFilter->setDateTime(dateTime);
    }

    connect(dateTimeFilter, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(on_dateTimeFilter_dateTimeChanged(QDateTime)));
    editFilterTimer.start(1000);
}

void BookedAtcDialog::on_editFilter_textChanged(QString searchStr)
{
    editFilterTimer.start(1000);
}

void BookedAtcDialog::on_spinHours_valueChanged(int val)
{
    editFilterTimer.start(1000);
}

void BookedAtcDialog::performSearch() {
    editFilterTimer.stop();

    // Text
    qDebug() << "BookedAtcDialog/performSearch(): building RegExp";
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
    QDateTime from = dateTimeFilter->dateTime();
    QDateTime to = from.addSecs(spinHours->value() * 3600);
    qDebug() << "BookedAtcDialog/performSearch(): setting Date" << from << to;
    bookedAtcSortModel->setDateTimeRange(from, to);

    // General
    //qDebug() << "BookedAtcDialog/performSearch(): resizing headers";
    //treeBookedAtc->header()->resizeSections(QHeaderView::ResizeToContents);
    boxResults->setTitle(QString("Results (%1)").arg(bookedAtcSortModel->rowCount()));
    qDebug() << "BookedAtcDialog/performSearch() -- finished";
}

void BookedAtcDialog::modelSelected(const QModelIndex& index) {
    bookedAtcModel.modelSelected(bookedAtcSortModel->mapToSource(index));
}

void BookedAtcDialog::on_tbPredict_clicked()
{
    close();
    Whazzup::getInstance()->setPredictedTime(dateTimeFilter->dateTime());
}
