/**************************************************************************
 *  This file is part of QuteScoop.
 *  Copyright (C) 2007-2008 Martin Domig <martin@domig.net>
 *
 *  QuteScoop is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  QuteScoop is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with QuteScoop.  If not, see <http://www.gnu.org/licenses/>
 **************************************************************************/

#include "BookedAtcDialog.h"
#include "BookedAtcDialogModel.h"
#include "BookedAtcSortFilter.h"
#include "Whazzup.h"
#include "Window.h"
#include "Settings.h"
#include "ui_MainWindow.h"

BookedAtcDialog *bookedAtcDialog = 0;

BookedAtcDialog *BookedAtcDialog::getInstance() {
	if(bookedAtcDialog == 0)
		bookedAtcDialog = new BookedAtcDialog();
	return bookedAtcDialog;
}


BookedAtcDialog::BookedAtcDialog() :
    QDialog()
{
    setupUi(this);
    setWindowFlags(Qt::Tool);

   	//bookedAtcSortModel = new QSortFilterProxyModel;
   	bookedAtcSortModel = new BookedAtcSortFilter;
	bookedAtcSortModel->setDynamicSortFilter(true);
	bookedAtcSortModel->setSourceModel(&bookedAtcModel);
	treeBookedAtc->setModel(bookedAtcSortModel);
   
    treeBookedAtc->header()->setResizeMode(QHeaderView::Interactive);
    treeBookedAtc->sortByColumn(4, Qt::AscendingOrder); 

    connect(treeBookedAtc->header(), SIGNAL(sectionClicked(int)), treeBookedAtc, SLOT(sortByColumn(int)));
    connect(treeBookedAtc, SIGNAL(clicked(const QModelIndex&)), &bookedAtcModel, SLOT(modelSelected(const QModelIndex&)));
    
    dateFilter->setDate(QDateTime::currentDateTime().toUTC().date());
    //dateFilter->setMinimumDate(QDateTime::currentDateTime().toUTC().date().addDays(-1));
    //dateFilter->setMaximumDate(QDateTime::currentDateTime().date().addMonths(1));
    timeFilter->setTime(QDateTime::currentDateTime().toUTC().time());

    QFont font = lblStatusInfo->font();
    font.setPointSize(lblStatusInfo->fontInfo().pointSize() - 1);
    lblStatusInfo->setFont(font); //make it a bit smaller than standard text

    connect(this, SIGNAL(needBookings()), Whazzup::getInstance(), SLOT(downloadBookings()));

    refresh();
}

void BookedAtcDialog::refresh() {
    if(Settings::downloadBookings() && !Whazzup::getInstance()->realWhazzupData().bookingsTimestamp().isValid())
        emit needBookings();

    bookedAtcModel.setClients(Whazzup::getInstance()->realWhazzupData().getBookedControllers());
    treeBookedAtc->header()->resizeSections(QHeaderView::ResizeToContents);
    bookedAtcSortModel->invalidate();

    const WhazzupData &data = Whazzup::getInstance()->realWhazzupData();

    QString msg = QString("Bookings %1 updated")
                  .arg(data.bookingsTimestamp().date() == QDateTime::currentDateTime().toUTC().date() // is today?
                        ? QString("today %1").arg(data.bookingsTimestamp().time().toString())
                        : (data.bookingsTimestamp().isValid()
                           ? data.bookingsTimestamp().toString("ddd yyyy/MM/dd HH:mm:ss")
                           : "never")
                        );
    lblStatusInfo->setText(msg);
}

void BookedAtcDialog::on_editFilter_textChanged(QString searchStr)
{
    QRegExp regex;
   	QStringList tokens = searchStr.trimmed().replace(QRegExp("\\*"), ".*").split(QRegExp("[ \\,]+"), QString::SkipEmptyParts);
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

    bookedAtcSortModel->setFilterRegExp(regex);
    bookedAtcSortModel->setFilterKeyColumn(-1);
}

void BookedAtcDialog::on_spinHours_valueChanged(int val)
{    
    QDateTime from = QDateTime(dateFilter->date(), timeFilter->time(), Qt::UTC);
    QDateTime to = from.addSecs(spinHours->value() * 3600);
    bookedAtcSortModel->setDateTimeRange(from, to);
    treeBookedAtc->header()->resizeSections(QHeaderView::ResizeToContents);
}


void BookedAtcDialog::on_timeFilter_timeChanged(QTime date)
{
    QDateTime from = QDateTime(dateFilter->date(), timeFilter->time(), Qt::UTC);
    QDateTime to = from.addSecs(spinHours->value() * 3600);
    bookedAtcSortModel->setDateTimeRange(from, to);
    treeBookedAtc->header()->resizeSections(QHeaderView::ResizeToContents);
}

void BookedAtcDialog::on_dateFilter_dateChanged(QDate date)
{
    QDateTime from = QDateTime(dateFilter->date(), timeFilter->time(), Qt::UTC);
    QDateTime to = from.addSecs(spinHours->value() * 3600);
    bookedAtcSortModel->setDateTimeRange(from, to);
    treeBookedAtc->header()->resizeSections(QHeaderView::ResizeToContents);
}

void BookedAtcDialog::modelSelected(const QModelIndex& index) {
	bookedAtcModel.modelSelected(bookedAtcSortModel->mapToSource(index));
}

void BookedAtcDialog::on_tbPredict_clicked()
{
    hide();
    Whazzup::getInstance()->setPredictedTime(QDateTime(dateFilter->date(), timeFilter->time(), Qt::UTC));
}
