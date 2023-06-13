#include "Airac.h"
#include "Launcher.h"
#include "NavData.h"
#include "Platform.h"
#include "Settings.h"
#include "src/Airport.h"

#include <QApplication>
#include <QMessageBox>
#include <QtCore>

/* logging */
QScopedPointer<QFile> m_logFile;

void messageHandler(QtMsgType type, const QMessageLogContext& a, const QString& msg) {
    const QMap<QtMsgType, QString> typeStrings {
        { QtInfoMsg, "INFO" },
        { QtDebugMsg, "DBG" },
        { QtWarningMsg, "WARN" },
        { QtCriticalMsg, "CRIT" },
        { QtFatalMsg, "FATAL" },
    };

    const QString function(a.function);
    const auto line = QString("[%1] %2 +%5 %4 %3").arg(
        typeStrings.value(type),
        QFileInfo(a.file).fileName(),
        msg,
        function
    ).arg(a.line);

    QTextStream stdoutStream(stdout);

    // useful for ad-hoc stdout debugging with all the nice QDebug type conversions, too
    if (type == QtCriticalMsg || type == QtFatalMsg) {
        stdoutStream << line << Qt::endl;
    }

    QTextStream out(m_logFile.data());
    out << QDateTime::currentDateTimeUtc().toString("HH:mm:ss.zzz[Z]") << " " << line << Qt::endl;
    out.flush();
}


/* main */
int main(int argc, char* argv[]) {
    QApplication app(argc, argv); // before QT_REQUIRE_VERSION to prevent creating duplicate..
    QT_REQUIRE_VERSION(argc, argv, "5.15.0"); // ..application objects

    // some app init
    app.setOrganizationName("QuteScoop");
    app.setOrganizationDomain("qutescoop.github.io");
    app.setApplicationName("QuteScoop");
    app.setApplicationVersion(Platform::version());
    app.setWindowIcon(QIcon(QPixmap(":/icons/qutescoop.png")));
    //QLocale::setDefault(QLocale::C); // bullet-proof string->float conversions

    // this can be removed in Qt6 https://bugreports.qt.io/browse/QTBUG-70431
    QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);

    // Open log.txt
    m_logFile.reset(new QFile(Settings::dataDirectory("log.txt")));
    m_logFile.data()->open(QFile::WriteOnly | QFile::Text);

    // catch all messages
    qInstallMessageHandler(messageHandler);

    // stdout
    QTextStream(stdout) << "Log output can be found in " << Settings::dataDirectory("log.txt") << '\n';
    QTextStream(stdout) << "Using settings from " << Settings::fileName() << '\n';

    // some initial debug logging
    qDebug().noquote() << "QuteScoop" << Platform::version();
    qDebug() << "Using settings from" << Settings::fileName();
    qDebug() << "Using application data from" << Settings::dataDirectory();

    qDebug().noquote() << QString("Compiled with Qt %1 [%4, mode: %2], running with Qt %3 on %5.").arg(
        QT_VERSION_STR, Platform::compileMode(), qVersion(), Platform::compiler(), Platform::platformOS()
        );

    // image format plugins
    app.addLibraryPath(QString("%1/imageformats").arg(app.applicationDirPath()));
    qDebug() << "Library paths:" << app.libraryPaths();
    qDebug() << "Supported image formats:" << QImageReader::supportedImageFormats();

    // command line parameters
    QCommandLineParser parser;
    parser.addHelpOption();

    QCommandLineOption routeOption(
        { "r", "route" },
        "resolve route",
        "<dep> <route> <dest>"

    );
    parser.addOptions({ routeOption });
    parser.process(app);
    if (parser.isSet(routeOption)) { // resolves a route
        if (parser.positionalArguments().size() < 1) {
            QTextStream(stdout) << "ERROR: need at least 2 arguments" << Qt::endl;
            return 1;
        }

        auto navData = NavData::instance();
        navData->load();
        auto airac = Airac::instance();
        airac->load();

        const auto depString = parser.value(routeOption);
        QStringList route = parser.positionalArguments();
        route.prepend(depString);
        QTextStream(stdout) << "\n# Flightplan route:\n" << route.join(" ") << Qt::endl;

        const auto dep = NavData::instance()->airports[depString];
        if (dep == 0) {
            QTextStream(stdout) << "ERROR: departure aerodrome not found in database" << Qt::endl;
            return 1;
        }

        const auto waypoints = Airac::instance()->resolveFlightplan(route, dep->lat, dep->lon);

        QTextStream(stdout) << "\n# Waypoints:" << Qt::endl;
        foreach (const auto &w, waypoints) {
            QTextStream(stdout) << w->id
                                << "\n"
                                << "\t" << "location: "
                                << QString("%1 (%2)").arg(NavData::toEurocontrol(w->lat, w->lon), QString("%1/%2").arg(w->lat).arg(w->lon))
                                << Qt::endl
            ;
            QStringList airways;
            foreach (const auto &awyList, airac->airways) {
                foreach (const auto &a, awyList) {
                    if (a->waypoints().contains(w)) {
                        airways << a->name;
                    }
                }
            }
            if (!airways.isEmpty()) {
                QTextStream(stdout) << "\tairways: " << airways.join(", ") << Qt::endl;
            }

        }

        return 0;
    }

    // show Launcher
    Launcher::instance()->fireUp();

    QMessageLogger("ile.constData()", 43, 0).debug() << "lkiij";

    // start event loop
    int ret = app.exec();

    return ret;
}
