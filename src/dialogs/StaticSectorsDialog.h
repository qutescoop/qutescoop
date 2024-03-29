#ifndef STATICSECTORSDIALOG_H
#define STATICSECTORSDIALOG_H

#include "ui_StaticSectorsDialog.h"

class StaticSectorsDialog
    : public QDialog, public Ui::StaticSectorsDialog {
    Q_OBJECT
    public:
        static StaticSectorsDialog* instance(bool createIfNoInstance = true, QWidget* parent = 0);
    protected:
        void closeEvent(QCloseEvent* event);
    private slots:
        void btnSelectAllTriggered();
        void btnSelectNoneTriggered();
        void itemChanged();
        void itemDoubleClicked(QListWidgetItem* item);

    private:
        StaticSectorsDialog(QWidget* parent = 0);
        void loadSectorList();

        constexpr const static char m_preferencesName[] = "StaticSectorsDialog";
};

#endif
