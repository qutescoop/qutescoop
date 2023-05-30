#include "BookedAtcDialog.h"

#include "Window.h"
#include "../BookedAtcDialogModel.h"
#include "../BookedAtcSortFilter.h"
#include "../Settings.h"
#include "../Whazzup.h"

// singleton instance
BookedAtcDialog* bookedAtcDialogInstance = 0;
BookedAtcDialog* BookedAtcDialog::instance(bool createIfNoInstance, QWidget* parent) {
    if(bookedAtcDialogInstance == 0) {
        if(createIfNoInstance) {
            if(parent == 0) {
                parent = Window::instance();
            }
            bookedAtcDialogInstance = new BookedAtcDialog(parent);
        }
    }
    return bookedAtcDialogInstance;
}

// destroys a singleton instance
void BookedAtcDialog::destroyInstance() {
    delete bookedAtcDialogInstance;
    bookedAtcDialogInstance = 0;
}


BookedAtcDialog::BookedAtcDialog(QWidget* parent) :
    QDialog(parent) {
    setupUi(this);
    setWindowFlags(windowFlags() ^= Qt::WindowContextHelpButtonHint);

    _bookedAtcModel = new BookedAtcDialogModel;
    _bookedAtcSortModel = new BookedAtcSortFilter;
    _bookedAtcSortModel->setDynamicSortFilter(true);
    _bookedAtcSortModel->setSourceModel(_bookedAtcModel);
    treeBookedAtc->setUniformRowHeights(true);
    treeBookedAtc->setModel(_bookedAtcSortModel);
    treeBookedAtc->sortByColumn(0, Qt::AscendingOrder);

    // disconnect to set DateTime without being disturbed
    // fixes https://sourceforge.net/p/qutescoop/tickets/5/
    disconnect(dateTimeFilter, &QDateTimeEdit::dateTimeChanged, this, &BookedAtcDialog::on_dateTimeFilter_dateTimeChanged);
    _dateTimeFilter_old = QDateTime::currentDateTimeUtc();
    dateTimeFilter->setDateTime(_dateTimeFilter_old);
    connect(dateTimeFilter, &QDateTimeEdit::dateTimeChanged, this, &BookedAtcDialog::on_dateTimeFilter_dateTimeChanged);
    connect(&_editFilterTimer, &QTimer::timeout, this, &BookedAtcDialog::performSearch);

    QFont font = lblStatusInfo->font();
    font.setPointSize(lblStatusInfo->fontInfo().pointSize() - 1);
    lblStatusInfo->setFont(font); //make it a bit smaller than standard text

    connect(this, &BookedAtcDialog::needBookings, Whazzup::instance(), &Whazzup::downloadBookings);

    auto preferences = Settings::dialogPreferences(m_preferencesName);
    if(!preferences.size.isNull()) {
        resize(preferences.size);
    }
    if(!preferences.pos.isNull()) {
        move(preferences.pos);
    }
    if(!preferences.geometry.isNull()) {
        restoreGeometry(preferences.geometry);
    }

    refresh();
}

void BookedAtcDialog::refresh() {
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));

    if(
        Settings::downloadBookings()
        && !Whazzup::instance()->realWhazzupData().bookingsTime.isValid()
    )
    {
        emit needBookings();
    }

    const WhazzupData &data = Whazzup::instance()->realWhazzupData();

    qDebug() << "BookedAtcDialog/refresh(): setting clients";
    _bookedAtcModel->setClients(data.bookedControllers);

    QString msg = QString("Bookings %1 updated")
        .arg(
        data.bookingsTime.date() == QDateTime::currentDateTimeUtc().date() // is today?
                        ? QString("today %1").arg(data.bookingsTime.time().toString("HHmm'z'"))
                        : (data.bookingsTime.isValid()
                           ? data.bookingsTime.toString("ddd MM/dd HHmm'z'")
                           : "never")
        );
    lblStatusInfo->setText(msg);
    _editFilterTimer.start(5);
    qApp->restoreOverrideCursor();
    qDebug() << "BookedAtcDialog/refresh() -- finished";
}

void BookedAtcDialog::setDateTime(const QDateTime &dateTime)
{
    dateTimeFilter->setDateTime(dateTime);
}

void BookedAtcDialog::on_dateTimeFilter_dateTimeChanged(QDateTime dateTime) {
    // some niceify on the default behaviour, making the sections depend on each other
    disconnect(dateTimeFilter, &QDateTimeEdit::dateTimeChanged, this, &BookedAtcDialog::on_dateTimeFilter_dateTimeChanged);
    _editFilterTimer.stop();

    // make year change if M 12+ or 0-
    if(
        (_dateTimeFilter_old.date().month() == 12)
        && (dateTime.date().month() == 1)
    )
    {
        dateTime = dateTime.addYears(1);
    }
    if(
        (_dateTimeFilter_old.date().month() == 1)
        && (dateTime.date().month() == 12)
    )
    {
        dateTime = dateTime.addYears(-1);
    }

    // make month change if d lastday+ or 0-
    if(
        (_dateTimeFilter_old.date().day() == _dateTimeFilter_old.date().daysInMonth())
        && (dateTime.date().day() == 1)
    )
    {
        dateTime = dateTime.addMonths(1);
    }
    if(
        (_dateTimeFilter_old.date().day() == 1)
        && (dateTime.date().day() == dateTime.date().daysInMonth())
    )
    {
        dateTime = dateTime.addMonths(-1);
        dateTime = dateTime.addDays(
            // compensate for month lengths
            dateTime.date().daysInMonth()
            - _dateTimeFilter_old.date().daysInMonth()
        );
    }

    // make day change if h 23+ or 00-
    if(
        (_dateTimeFilter_old.time().hour() == 23)
        && (dateTime.time().hour() == 0)
    )
    {
        dateTime = dateTime.addDays(1);
    }
    if(
        (_dateTimeFilter_old.time().hour() == 0)
        && (dateTime.time().hour() == 23)
    )
    {
        dateTime = dateTime.addDays(-1);
    }

    // make hour change if m 59+ or 0-
    if(
        (_dateTimeFilter_old.time().minute() == 59)
        && (dateTime.time().minute() == 0)
    )
    {
        dateTime = dateTime.addSecs(60 * 60);
    }
    if(
        (_dateTimeFilter_old.time().minute() == 0)
        && (dateTime.time().minute() == 59)
    )
    {
        dateTime = dateTime.addSecs(-60 * 60);
    }

    _dateTimeFilter_old = dateTime;

    if(
        dateTime.isValid()
        && (dateTime != dateTimeFilter->dateTime())
    )
    {
        dateTimeFilter->setDateTime(dateTime);
    }

    connect(dateTimeFilter, &QDateTimeEdit::dateTimeChanged, this, &BookedAtcDialog::on_dateTimeFilter_dateTimeChanged);
    _editFilterTimer.start(1000);
}

void BookedAtcDialog::on_editFilter_textChanged(QString) {
    _editFilterTimer.start(1000);
}

void BookedAtcDialog::on_spinHours_valueChanged(int) {
    _editFilterTimer.start(1000);
}

void BookedAtcDialog::performSearch() {
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
    _editFilterTimer.stop();

    // Text
    qDebug() << "BookedAtcDialog/performSearch(): building RegExp";
    QRegExp regex;
    // @todo this tries to cater for both ways (wildcards and regexp) but it does a bad job at that.
    QStringList tokens = editFilter->text()
        .replace(QRegExp("\\*"), ".*")
        .split(QRegExp("[ \\,]+"), Qt::SkipEmptyParts);
    if(tokens.size() == 1) {
        regex = QRegExp("^" + tokens.first() + ".*", Qt::CaseInsensitive);
    } else if(tokens.size() == 0) {
        regex = QRegExp("");
    } else {
        QString regExpStr = "^(" + tokens.first();
        for(int i = 1; i < tokens.size(); i++) {
            regExpStr += "|" + tokens[i];
        }
        regExpStr += ".*)";
        regex = QRegExp(regExpStr, Qt::CaseInsensitive);
    }

    _bookedAtcSortModel->setFilterKeyColumn(-1);
    _bookedAtcSortModel->setFilterRegExp(regex);

    //Date, Time, TimeSpan
    QDateTime from = dateTimeFilter->dateTime().toUTC();
    QDateTime to = from.addSecs(spinHours->value() * 3600);
    _bookedAtcSortModel->setDateTimeRange(from, to);

    boxResults->setTitle(QString("Results (%1)").arg(_bookedAtcSortModel->rowCount()));
    qApp->restoreOverrideCursor();
    qDebug() << "BookedAtcDialog/performSearch() -- finished";
}

void BookedAtcDialog::on_tbPredict_clicked() {
    close();
    Window::instance()->actionPredict->setChecked(true);
    Window::instance()->dateTimePredict->setDateTime(
        dateTimeFilter->dateTime()
    );
}

void BookedAtcDialog::closeEvent(QCloseEvent* event) {
    Settings::setDialogPreferences(
        m_preferencesName,
        Settings::DialogPreferences {
            .size = size(),
            .pos = pos(),
            .geometry = saveGeometry()
        }
    );
    event->accept();
}
