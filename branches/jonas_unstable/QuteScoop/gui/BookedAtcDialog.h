/**************************************************************************
 *  This file is part of QuteScoop.
 *  Copyright (C) 2007-2009 Martin Domig <martin@domig.net>
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

#ifndef BOOKEDATCDIALOG_H
#define BOOKEDATCDIALOG_H

#include <QSortFilterProxyModel>

#include "BookedAtcDialogModel.h"
#include "BookedAtcSortFilter.h"
#include "ui_BookedAtcDialog.h"

class BookedAtcDialog : public QDialog, private Ui::BookedAtcDialog {
    Q_OBJECT
public:
    BookedAtcDialog();
	static BookedAtcDialog *getInstance();
    void refresh();
    
private slots:
    void modelSelected(const QModelIndex& index);
    void on_dateFilter_dateChanged(QDate date);
    void on_timeFilter_timeChanged(QTime date);
    void on_spinHours_valueChanged(int val);
    void on_editFilter_textChanged(QString str);

private:
    BookedAtcDialogModel bookedAtcModel;
	BookedAtcSortFilter *bookedAtcSortModel;
};

#endif // BOOKEDATCDIALOG_H
