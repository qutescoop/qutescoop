/**************************************************************************
*  This file is part of QuteScoop. See README for license
**************************************************************************/

#include "StaticSectorsDialog.h"

#include "Window.h"
#include "../MapScreen.h"
#include "../NavData.h"
#include "../Settings.h"

//singleton instance
StaticSectorsDialog* staticSectorsDialogInstance = 0;
StaticSectorsDialog* StaticSectorsDialog::instance(bool createIfNoInstance, QWidget* parent) {
    if (staticSectorsDialogInstance == 0 && createIfNoInstance) {
        staticSectorsDialogInstance = new StaticSectorsDialog(parent);
    }
    return staticSectorsDialogInstance;
}

StaticSectorsDialog::StaticSectorsDialog(QWidget* parent) :
    QDialog(parent) {
    setupUi(this);

    setModal(false);
    setWindowFlags(windowFlags() ^= Qt::WindowContextHelpButtonHint);

    auto preferences = Settings::dialogPreferences(m_preferencesName);
    if (!preferences.size.isNull()) { resize(preferences.size); }
    if (!preferences.pos.isNull()) { move(preferences.pos); }
    if (!preferences.geometry.isNull()) { restoreGeometry(preferences.geometry); }

    connect(listWidgetSectors, &QListWidget::itemChanged, this, &StaticSectorsDialog::itemChanged);
    connect(btnSelectAll, &QPushButton::clicked, this, &StaticSectorsDialog::btnSelectAllTriggered);
    connect(btnSelectNone, &QPushButton::clicked, this, &StaticSectorsDialog::btnSelectNoneTriggered);

    loadSectorList();
}

void StaticSectorsDialog::loadSectorList() {
    qDebug() << "StaticSectorsDialog::loadSectorList -- started";
    foreach (auto sector, NavData::instance()->sectors.values()) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(
         QString("%1 %2 (ID %3, %4)").arg(
         sector->icao,
         (sector->controllerSuffixes().isEmpty()
                                ? ""
                                : "(" + sector->controllerSuffixes().join(", ") + ")"
         ),
         sector->id,
         sector->name
         )
        );
        item->setToolTip(
         QString(
         "controller prefix: %1\n"
         "controller suffixes: %2\n"
         "sector ID: %3\n"
         "sector name: %4\n"
         "number of polygon points: %5\n"
         "controller defined in line #%6 in data/firlist.dat\n"
         "sector defined in line #%7 in data/firdisplay.dat"
         )
         .arg(
         sector->icao,
         (sector->controllerSuffixes().isEmpty()
                        ? "[any]"
                        : sector->controllerSuffixes().join("/")
         ),
         sector->id,
         sector->name
         )
         .arg(sector->points().size())
         .arg(sector->debugControllerLineNumber)
         .arg(sector->debugSectorLineNumber)
        );
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);

        item->setData(Qt::UserRole, QVariant::fromValue(static_cast<void*>(sector)));

        listWidgetSectors->addItem(item);
    }
    listWidgetSectors->sortItems();
    qDebug() << "StaticSectorsDialog::loadSectorList -- finsished";
}

void StaticSectorsDialog::closeEvent(QCloseEvent* event) {
    Window::instance()->mapScreen->glWidget->setStaticSectors(QList<Sector*>());

    Settings::setDialogPreferences(
        m_preferencesName,
        Settings::DialogPreferences {
            .size = size(),
            .pos = pos(),
            .geometry = saveGeometry()
        }
    );

    event->accept();

    deleteLater();
    staticSectorsDialogInstance = 0;
}

void StaticSectorsDialog::btnSelectAllTriggered()
{
    foreach (auto item, listWidgetSectors->findItems("", Qt::MatchStartsWith)) {
        item->setCheckState(Qt::Checked);
    }
}

void StaticSectorsDialog::btnSelectNoneTriggered()
{
    foreach (auto item, listWidgetSectors->findItems("", Qt::MatchStartsWith)) {
        item->setCheckState(Qt::Unchecked);
    }
}

void StaticSectorsDialog::itemChanged() {
    QList<Sector*> renderSectors;

    foreach (auto item, listWidgetSectors->findItems("", Qt::MatchStartsWith)) {
        if (item->checkState() == Qt::Checked) {
            auto sector = static_cast<Sector*>(item->data(Qt::UserRole).value<void*>());
            if (sector != 0) {
                renderSectors.append(sector);
            }
        }
    }

    Window::instance()->mapScreen->glWidget->setStaticSectors(renderSectors);
}
