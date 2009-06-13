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

#include "PlanFlightDialog.h"
#include "ui_PlanFlightDialog.h"

#include <QDebug>
#include <QtCore>

#include "Settings.h"

PlanFlightDialog *planFlightDialogInstance = 0;

PlanFlightDialog *PlanFlightDialog::getInstance() {
	if(planFlightDialogInstance == 0)
		planFlightDialogInstance = new PlanFlightDialog();
	return planFlightDialogInstance;
}

PlanFlightDialog::PlanFlightDialog():
    QDialog()
{
    setupUi(this);
    
    QUrl url("http://vatbook.euroutepro.com/servinfo.asp");//"http://www.euroutepro.com/fp/fp_main.php?arr=eddf&mode=sel1&dep=lfpg");
    QFileInfo fileInfo(url.path());
    QString fileName = fileInfo.fileName();

    //if(fpDownloader != 0) {
    //    fpDownloader->abort();
    //    delete fpDownloader;
    //}
    fpDownloader = new QHttp;
    fpBuffer = 0;

    connect(fpDownloader, SIGNAL(done(bool)), this, SLOT(fpDownloaded(bool)));
    Settings::applyProxySetting(fpDownloader);

    fpDownloader->setHost(url.host(), url.port() != -1 ? url.port() : 80);
    if (!url.userName().isEmpty())
        fpDownloader->setUser(url.userName(), url.password());

    QString querystr = url.path() + "?" + url.encodedQuery();

    if(fpBuffer != 0) delete fpBuffer;
    fpBuffer = new QBuffer;
    fpBuffer->open(QBuffer::ReadWrite);
    fpDownloader->get(querystr, fpBuffer);
    
    
}

void PlanFlightDialog::fpDownloaded(bool error) {
    if(fpBuffer == 0)
        return;

    if(error) {
        emit downloadError(fpDownloader->errorString());
        return;
    }

    fpBuffer->seek(0);
    while(fpBuffer->canReadLine()) {
        QString line = QString(fpBuffer->readLine()).trimmed();
        QStringList list = line.split('=');
    }

    delete fpBuffer;
    fpBuffer = 0;

    emit fpDownloaded();
}
