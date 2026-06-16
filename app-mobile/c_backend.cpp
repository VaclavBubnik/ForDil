/*
    Copyright 2025 - Present MENDELU

    This file is part of ForDil.

    ForDil is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ForDil is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ForDil. If not, see <https://www.gnu.org/licenses/>.
*/

#include "c_backend.h"
#include "../backend/c_chart.h"
#include "../backend/c_mappainter.h"
#include "../backend/c_result.h"
#include "../backend/c_pdf.h"
#include <QSettings>
#include <QScreen>
#include <QFile>
#include <QStandardPaths>
#include <QSaveFile>
#include <QDir>

static Backend *g_backend = nullptr;
static QtMessageHandler g_prevHandler = nullptr;

static void backendLogHandler(QtMsgType type,
                              const QMessageLogContext &context,
                              const QString &msg)
{
    QString level;
    switch (type) {
    case QtDebugMsg:    level = "DBG";
    case QtInfoMsg:     level = "INF";
    case QtWarningMsg:  level = "WAR";
    case QtCriticalMsg: level = "ERR";
    case QtFatalMsg:    level = "FAT";
    default: level = "LOG";
    }

    QString text = msg;

    if (g_backend)
    {
        QMetaObject::invokeMethod(
            g_backend,
            [level, text]() {
                g_backend->appendLogLine(level, text);
            },
            Qt::QueuedConnection
        );
    }

    if (g_prevHandler)
    {
        g_prevHandler(type, context, msg);
    }
}

Backend::ImageProvider::ImageProvider(Backend *backend)
    : QQuickImageProvider(QQuickImageProvider::Image),
    m_backend(backend)
{
    qDebug() << "Backend ImageProvider created";
}

QImage Backend::ImageProvider::requestImage(const QString &imageId, QSize *size, const QSize &requestedSize)
{
    return m_backend->requestImage(imageId, size, requestedSize);
}

Backend::Backend(QApplication *application, QObject *parent)
  : QObject(parent),
    m_application(application),
    m_imageProvider(this)
{
    g_backend = this;
    g_prevHandler = qInstallMessageHandler(backendLogHandler);

    qDebug() << "Backend created";
    initCurveOptions();
}

// Init
void Backend::init()
{
}

void Backend::setupWindow()
{
    QScreen *screen = QApplication::primaryScreen();
    if (screen)
    {
        int screenWidth = screen->geometry().width();
        qDebug() << "screenWidth" << screenWidth;
        int screenHeight = screen->geometry().height();
        qDebug() << "screenHeight" << screenHeight;
        m_data.setupWindow(screenWidth, screenHeight);
    }
    else
    {
        qDebug() << "screen is not ready yet";
    }
}

// Log
void Backend::appendLogLine(const QString &level, const QString &message)
{
    const QString line =
        QString("[%1] %2  %3")
            .arg(QDateTime::currentDateTime().toString("HH:mm:ss"),
                 level,
                 message);

    m_logLines.append(line);

    constexpr int maxLines = 500;
    while (m_logLines.size() > maxLines)
    {
        m_logLines.removeFirst();
    }

    emit logLinesChanged();
}

// Settings
void Backend::setSetting(const QString &key, const QVariant &value)
{
#if defined(FORDIL_DEBUG_BACKEND)
    qDebug() << "setSetting" << key << "value" << value;
#endif
    QSettings settings;
    settings.setValue(key, value);
}

QVariant Backend::getSetting(const QString &key, const QVariant &defaultValue)
{
    QSettings settings;
    QVariant value = settings.value(key, defaultValue);
#if defined(FORDIL_DEBUG_BACKEND)
    qDebug() << "getSetting" << key << "value" << value;
#endif
    return value;
}

// Settings Misc
bool Backend::isDebugBuild() const
{
#ifdef QT_DEBUG
    return true;
#else
    return false;
#endif
}

bool Backend::isDebugModeEnabled()
{
    return getSetting("debugModeEnabled", "false").toBool();
}

void Backend::setDebugModeEnabled(bool enabled)
{
    if (isDebugModeEnabled() != enabled)
    {
#if defined(FORDIL_DEBUG_BACKEND)
        qDebug() << "setDebugModeEnabled" << (enabled ? "Y" : "N");
#endif
        setSetting("debugModeEnabled", enabled);
        emit isDebugModeEnabledChanged();
    }
}

bool Backend::isDebugQmlEnabled()
{
    return false;
}

// Settings GPS
bool Backend::isGpsEnabled()
{
    return getSetting("gpsEnabled", "true").toBool();
}

void Backend::setGpsEnabled(bool enabled)
{
    if (isGpsEnabled() != enabled)
    {
#if defined(FORDIL_DEBUG_BACKEND)
        qDebug() << "setGpsEnabled" << (enabled ? "Y" : "N");
#endif
        setSetting("gpsEnabled", enabled);
        emit isGpsEnabledChanged();
    }
}

bool Backend::isShowGpsCoordinatesEnabled()
{
    return getSetting("showGpsCoordinatesEnabled", "false").toBool();
}

void Backend::setShowGpsCoordinatesEnabled(bool enabled)
{
    if (isShowGpsCoordinatesEnabled() != enabled)
    {
#if defined(FORDIL_DEBUG_BACKEND)
        qDebug() << "setShowGpsCoordinatesEnabled" << (enabled ? "Y" : "N");
#endif
        setSetting("showGpsCoordinatesEnabled", enabled);
        emit isShowGpsCoordinatesEnabledChanged();
    }
}

// Settings Tree selection
double Backend::treeScale()
{
    return getSetting("treeScale", "5.0").toDouble();
}

void Backend::setTreeScale(double scale)
{
#if defined(FORDIL_DEBUG_BACKEND)
    qDebug() << "setTreeScale" << scale;
#endif
    setSetting("treeScale", scale);
}

// Settings Results
bool Backend::isSpeciesInTreeStatisticsEnabled()
{
    return getSetting("showSpeciesInTreeStatistics", "false").toBool();
}

void Backend::setSpeciesInTreeStatisticsEnabled(bool enabled)
{
#if defined(FORDIL_DEBUG_BACKEND)
    qDebug() << "setSpeciesInTreeStatisticsEnabled" << (enabled ? "Y" : "N");
#endif
    setSetting("showSpeciesInTreeStatistics", enabled);
}

bool Backend::isRoundValuesEnabled()
{
    return false;
    //return getSetting("roundValues", "true").toBool();
}

void Backend::setRoundValuesEnabled(bool enabled)
{
#if defined(FORDIL_DEBUG_BACKEND)
    qDebug() << "setRoundValuesEnabled" << (enabled ? "Y" : "N");
#endif
    setSetting("roundValues", enabled);
}

QString Backend::treeDistributionOption()
{
    return getSetting("treeDistributionOption", "NHA").toString();
}

void Backend::setTreeDistributionOption(const QString &option)
{
    if (treeDistributionOption() != option)
    {
#if defined(FORDIL_DEBUG_BACKEND)
        qDebug() << "setTreeDistributionOption" << option;
#endif
        setSetting("treeDistributionOption", option);
        emit treeDistributionOptionChanged();
    }
}

QString Backend::typologyLocalResultsSettings()
{
    return getSetting("typologyLocalResults", "User").toString();
}

void Backend::setTypologyLocalResultsSettings(const QString &option)
{
    if (typologyLocalResultsSettings() != option)
    {
#if defined(FORDIL_DEBUG_BACKEND)
        qDebug() << "setTypologyLocalResultsSettings" << option;
#endif
        setSetting("typologyLocalResults", option);
        emit typologyLocalResultsSettingsChanged();
    }
}

QString Backend::typologyGlobalResultsSettings()
{
    return getSetting("typologyGlobalResults", "0 years").toString();
}

void Backend::setTypologyGlobalResultsSettings(const QString &option)
{
    if (typologyGlobalResultsSettings() != option)
    {
#if defined(FORDIL_DEBUG_BACKEND)
        qDebug() << "setTypologyGlobalResultsSettings" << option;
#endif
        setSetting("typologyGlobalResults", option);
        emit typologyGlobalResultsSettingsChanged();
    }
}

void Backend::setShowMapTreeSpeciesLegend()
{
    bool b = m_data.view().showMapTreeSpeciesLegend();
    m_data.view().setShowMapTreeSpeciesLegend(!b);
}

// Language
QString Backend::systemLanguageCode() const
{
    QString systemLocale = QLocale::system().name(); // e.g., "en_US"
    QString localLanguageCode = systemLocale.split('_').first(); // "en"
#if defined(FORDIL_DEBUG_BACKEND)
    qDebug() << "systemLocale" << systemLocale << "localLanguageCode" << localLanguageCode;
#endif
    if (localLanguageCode != "en" &&
        localLanguageCode != "fr" &&
        localLanguageCode != "cs")
    {
        localLanguageCode = "en";
    }
    return localLanguageCode;
}

void Backend::setLanguage()
{
#if defined(FORDIL_DEBUG_BACKEND)
    qDebug() << "start initializing application language";
#endif
    QString systemCode = systemLanguageCode();
    QVariant settingCode = getSetting("languageCode", systemCode);
    setLanguageByCode(settingCode.toString());
#if defined(FORDIL_DEBUG_BACKEND)
    qDebug() << "finished initializing application language";
#endif
}

QString Backend::languageCode()
{
    return getSetting("languageCode", "en").toString();
}

void Backend::setLanguageByCode(const QString &code)
{
    m_application->removeTranslator(&m_translator);

    if (m_translator.load(":/translations/translation_" + code + ".qm"))
    {
        m_application->installTranslator(&m_translator);
        core().setLanguageByCode(code);
        m_data.loadText();
        qDebug() << "Translation" << code << "loaded";
    }
    else
    {
        qWarning() << "Unable to load" << code << "translation";
    }

    setSetting("languageCode", code);
}

QString Backend::text(const QString &identifier) const
{
    return m_data.text(identifier);
};

// Exercise
QString Backend::storeFile(const QString &path)
{
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appData);

    QFile src(path);
    if (!src.open(QIODevice::ReadOnly))
    {
        return QCoreApplication::translate("Backend", "Error: Cannot open the file.");
    }

    QString system;
    QString siteName;
    QString error;

    Data::inspectDataFile(path, system, siteName, error);

    if (!error.isEmpty())
    {
        return QCoreApplication::translate("Backend", "Error: ") + error;
    }

    QString fileName = system + " " + siteName;
    fileName.replace(" ", "_");
    fileName = fileName.toLower();

    QString targetPath = appData + "/" + fileName + ".xlsx";
    QFile dst(targetPath);

    // Overwrite if exists
    bool updated = false;
    if (dst.exists())
    {
        dst.remove();
        updated = true;
    }

    if (dst.open(QIODevice::WriteOnly))
    {
        dst.write(src.readAll());
        dst.close();
    }

    src.close();

    if (updated)
    {
        return QCoreApplication::translate("Backend", "Successfully updated site") + " '" + fileName + "'.";
    }

    return QCoreApplication::translate("Backend", "Successfully imported new site") + " '" + fileName + "'.";
}

static QList<QString> backendListStoredFiles(const QString &pattern)
{
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appData);

    // Collect all files matching pattern
    QStringList files = dir.entryList(QStringList() << pattern, QDir::Files, QDir::Name);

    QList<QString> fullPaths;
    for (const QString &f : files)
    {
        fullPaths << dir.filePath(f);
    }

    return fullPaths;
}

QVariantList Backend::siteNamesModelData(const QString &system, bool discovery)
{
    // List files
    QList<QString> files = backendListStoredFiles("*.xlsx");

    // Collect sites
    std::map<QString, QString> sites;
    for (const auto &filePath : files)
    {
#if defined(FORDIL_DEBUG_BACKEND)
        qDebug() << "found" << filePath;
#endif
        QString siteName = Data::validateDataFile(filePath, system, discovery);
        if (siteName.isEmpty())
        {
            continue;
        }

        if (sites.count(siteName) > 0)
        {
            QString fileName = QFileInfo(filePath).fileName();
            siteName += " (" + fileName + ")";
        }

        sites[siteName] = filePath;
    }

#if defined(FORDIL_BUNDLE_SITE)
    // Use resources.qrc when the files are not available
    if (sites.count("Lauzelle BE") < 1)
    {
        sites["Lauzelle BE"] = ""; 
    }

    if (sites.count("Mormal FR") < 1)
    {
        sites["Mormal FR"] = "";
    }

    if (sites.count("Pokojna CZ") < 1)
    {
        sites["Pokojna CZ"] = "";
    }
#endif

    // Return model data
    QVariantList list;
    for (auto &[k, v] : sites)
    {
        list << QVariantMap{{"name", k}, {"path", v}};
    }

    return list;
}

void Backend::loadExercise(const QString &filePath,
                           const QString &siteName,
                           const QString &system,
                           bool discovery,
                           bool reload)
{
#if defined(FORDIL_DEBUG_BACKEND)
    qDebug() << "loadExercise" << siteName;
#endif

    try
    {
        if (reload)
        {
            m_data.loadExercise(filePath, siteName, system, discovery);

            setIntroductionSiteVisible(!introductionSiteBody().isEmpty() || m_data.loadedDiscoveryPart());
            setIntroductionResultsVisible(true);
            setIntroductionGlobalResultsVisible(true);
            setIntroductionAdvancedResultsVisible(true);

            setExerciseFinished(false);
            resetZoom("mapImage");
        }

        resetInteraction();
    }
    catch (std::exception &e)
    {
        qWarning() << QString("Failed to read exercise: ") + e.what();
    }
    catch (...)
    {
        qWarning() << "Failed to read exercise";
    }
}

QString Backend::loadedExerciseName() const
{
    return m_data.loadedExerciseName();
}

QString Backend::loadedSystem() const
{
    return m_data.loadedSystem();
}

bool Backend::loadedDiscoveryPart() const
{
    return m_data.loadedDiscoveryPart();
}

bool Backend::isExerciseFinished() const
{
    return m_data.isExerciseFinished();
}

void Backend::setExerciseFinished(bool value)
{
    if (m_data.isExerciseFinished() != value)
    {
        m_data.setExerciseFinished(value);
#if defined(FORDIL_DEBUG_BACKEND)
        qDebug() << "setExerciseFinished" << (value ? "Y" : "N");
#endif
        emit isExerciseFinishedChanged();
    }
}

QString Backend::elapsedExerciseTime() const
{
    return m_data.elapsedTime();
}

// Introduction screens
bool Backend::isIntroductionAppVisible() const
{
    return m_showIntroductionApp;
}

void Backend::setIntroductionAppVisible(bool visible)
{
    m_showIntroductionApp = visible;
}

bool Backend::isIntroductionSiteVisible() const
{
    return m_showIntroductionSite;
}

void Backend::setIntroductionSiteVisible(bool visible)
{
    m_showIntroductionSite = visible;
}

QString Backend::introductionSiteHeader() const
{
    return m_data.introductionSiteHeader();
}

QString Backend::introductionSiteBody() const
{
    return m_data.introductionSiteBody();
}

bool Backend::isIntroductionResultsVisible() const
{
    return m_showIntroductionResults;
}

void Backend::setIntroductionResultsVisible(bool visible)
{
    m_showIntroductionResults = visible;
}

bool Backend::isIntroductionGlobalResultsVisible() const
{
    return m_showIntroductionGlobalResults;
}

void Backend::setIntroductionGlobalResultsVisible(bool visible)
{
    m_showIntroductionGlobalResults = visible;
}

bool Backend::isIntroductionAdvancedResultsVisible() const
{
    return m_showIntroductionAdvancedResults;
}

void Backend::setIntroductionAdvancedResultsVisible(bool visible)
{
    m_showIntroductionAdvancedResults = visible;
}

// Image provider
bool Backend::isRedrawAllowed()
{
    QDateTime now = QDateTime::currentDateTime();

    return m_lastDrawTime.secsTo(now) >= 1;
}

QImage Backend::requestImage(const QString &imageId, QSize *size, const QSize &requestedSize)
{
#if defined(FORDIL_DEBUG_BACKEND) || defined(FORDIL_DEBUG_IMAGE)
    qDebug() << "requestImage id" << imageId
             << "w" << requestedSize.width()
             << "h" << requestedSize.height();
#endif

    if (requestedSize.width() < 1 || requestedSize.height() < 1)
    {
        if (m_emptyImage.width() != 1 || m_emptyImage.height() != 1)
        {
            m_emptyImage = QImage(1, 1, QImage::Format_ARGB32);
            m_emptyImage.fill(Qt::transparent);
        }

        return m_emptyImage;
    }

    int w = requestedSize.width();
    int h = requestedSize.height();

    if (m_image.width() != w || m_image.height() != h)
    {
        m_image = QImage(w, h, QImage::Format_ARGB32);
    }
    m_image.fill(Qt::transparent);

    QPainter painter(&m_image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (imageId == "mapImage")
    {
        m_lastDrawTime = QDateTime::currentDateTime();

        int w = painter.device()->width();
        int h = painter.device()->height();
        QScreen *screen = QApplication::primaryScreen();
        m_data.view().setDimensions(w, h, screen->logicalDotsPerInch());
        m_data.view().setTreeScale(treeScale());
        MapPainter::draw(painter, m_data);
    }
    else
    {
        updateChartOptions();
        Chart::requestImage(painter, imageId, m_data, m_chartOptions);
    }

    if (size)
    {
        *size = m_image.size();
    }

    return m_image;
}

double Backend::imageHeight(const QString &imageId, double width)
{
    updateChartOptions();
    return Chart::imageHeight(width, imageId, m_data, m_chartOptions);
}

void Backend::updateChartOptions()
{
    m_chartOptions.treeDistributionType = treeDistributionOption();
    m_chartOptions.showAllTreeStatisticsSpecies = isSpeciesInTreeStatisticsEnabled();
    m_chartOptions.typologyLocalType = typologyLocalResultsSettings();
    m_chartOptions.typologyGlobalAge = typologyGlobalResultsSettings();

}

// PDF
QString Backend::copyResourceToUrl(const QString &resourcePath, const QUrl &targetUrl)
{
    if (!targetUrl.isValid())
    {
        return "Invalid target location.";
    }

    // Desktop: expect file:// URL. (See mobile note below.)
    // if (!targetUrl.isLocalFile())
    // {
    //     return "Invalid target location.";
    // }

    // 1) Read from qrc (read-only)
    QFile src(resourcePath); // e.g. ":/pdfs/report.pdf"
    if (!src.open(QIODevice::ReadOnly))
    {
        return QString("Cannot read bundled file: %1").arg(src.errorString());
    }

#if 0
    const QString targetPath = targetUrl.toLocalFile();

    // Ensure folder exists
    QFileInfo fi(targetPath);
    if (!QDir().mkpath(fi.absolutePath()))
    {
        return "Cannot create target directory.";
    }

    QSaveFile dst(targetPath);            // safer than QFile for saving
    if (!dst.open(QIODevice::WriteOnly))
    {
        return QString("Cannot write file: %1").arg(dst.errorString());
    }

    const QByteArray data = src.readAll();
    if (dst.write(data) != data.size())
    {
        return "Failed while writing file.";
    }

    if (!dst.commit())
    {
        return QString("Failed to finalize file: %1").arg(dst.errorString());
    }
#else
    // 2) Write to target (Android SAF often returns content://...)
    // For content://, do NOT use toLocalFile() and do NOT use QSaveFile.
    // Open via QFile using the URL string.
    const QString urlString = targetUrl.toString();

    // Optional: basic scheme check (helps produce nicer errors)
    const QString scheme = targetUrl.scheme().toLower();
    if (!scheme.isEmpty() && scheme != "file" && scheme != "content")
    {
        return QString("Unsupported target scheme: %1").arg(scheme);
    }

    QFile dst(urlString);
    if (!dst.open(QIODevice::WriteOnly))
    {
        return QString("Cannot write file: %1").arg(dst.errorString());
    }

    // 3) Stream copy (avoids loading whole file into memory)
    constexpr qint64 bufSize = 256 * 1024;
    QByteArray buffer;
    buffer.resize(bufSize);

    while (!src.atEnd())
    {
        const qint64 n = src.read(buffer.data(), buffer.size());
        if (n < 0)
        {
            return QString("Read failed: %1").arg(src.errorString());
        }

        qint64 written = 0;
        while (written < n)
        {
            const qint64 w = dst.write(buffer.constData() + written, n - written);
            if (w < 0)
            {
                return QString("Write failed: %1").arg(dst.errorString());
            }
            written += w;
        }
    }

    if (!dst.flush())
    {
        return QString("Failed to flush file: %1").arg(dst.errorString());
    }
#endif

    return QString();
}

QString Backend::glossaryPdfFileName()
{
    if (languageCode() == "fr")
    {
        return "Glossaire.pdf";
    }

    if (languageCode() == "cs")
    {
        return "Slovníček.pdf";
    }

    return "Glossary.pdf";
}

QString Backend::copyGlossaryToUrl(const QUrl &targetUrl)
{
    QString resourcePath;

    resourcePath = ":/data/" + glossaryPdfFileName();

    return copyResourceToUrl(resourcePath, targetUrl);
}

QString Backend::exportTravailloscopeQuizToPdf(const QUrl &targetUrl)
{
    return Pdf::exportTravailloscopeQuiz(targetUrl, m_data);
}

QString Backend::exportMarteloscopeResultsToPdf(const QUrl &targetUrl)
{
    updateChartOptions();
    return Pdf::exportMarteloscopeResults(targetUrl, m_data, m_chartOptions);
}

// Site map
double Backend::zoom(const QString &mapId)
{
    return m_data.view().zoom();
}

void Backend::setZoom(const QString &mapId, double value, double pinchScale, double x, double y, double w, double h)
{
    double rx = x / w;
    double ry = y / h;

#if defined(FORDIL_DEBUG_BACKEND)
    qDebug() << "setZoom x" << x << "y" << y << "w" << w << "h" << h << "pinchScale" << pinchScale;
    qDebug() << "startZoom rx" << rx << "ry" << ry;
#endif

    m_data.view().setZoom(value, pinchScale, rx, ry);
}

void Backend::zoomIn(const QString &mapId)
{
    setZoom(mapId, m_data.view().zoom() * 1.25, 1.0, 0.5, 0.5, 1.0, 1.0);
}

void Backend::zoomOut(const QString &mapId)
{
    setZoom(mapId, m_data.view().zoom() * 0.8, 1.0, 0.5, 0.5, 1.0, 1.0);
}

void Backend::resetZoom(const QString &mapId)
{
    setZoom(mapId, m_data.view().zoomMin(), 1.0, 0.5, 0.5, 1.0, 1.0);
    m_data.view().resetTranslation();
}

void Backend::translate(const QString &mapId, double dx, double dy)
{
    m_data.view().translate(dx, dy);
}

void Backend::translateToSelection(const QString &mapId)
{
    m_data.view().translateToSee(m_selectedTree.x(), m_selectedTree.y(), m_selectedTree.dbh(), treeScale(), 0.75);
}

bool Backend::translateToId(const QString &text)
{
    bool conversionSuccess{false};
    uint64_t id = text.toULongLong(&conversionSuccess);
    if (!conversionSuccess)
    {
        qWarning() << "Cannot convert text" << text << "to id in translateToId";
        return false;
    }

#if defined(FORDIL_DEBUG_BACKEND)
    qDebug() << "translateToId" << id << "text" << text;
#endif

    bool found{false};
    double x;
    double y;
    double r;

    if (loadedSystem() == "Marteloscope")
    {
        found = m_data.setTreePreselected(m_selectedTree, id);
        if (found)
        {
            x = m_selectedTree.x();
            y = m_selectedTree.y();
            r = m_selectedTree.dbh();
        }
    }
    else
    {
        found = m_data.setSpotPreselected(m_selectedSpot, id);
        if (found)
        {
            x = m_selectedSpot.x();
            y = m_selectedSpot.y();
            r = m_selectedSpot.radius();
        }
    }

    if (found)
    {
        m_data.view().translateToSee(x, y, r, treeScale());
    }

    return found;
}

void Backend::resetTranslation(const QString &mapId)
{
    m_data.view().resetTranslation();
}

void Backend::refocus(const QString &mapId)
{
    if (!isLocationAvailable())
    {
        return;
    }

    double x = m_data.view().locationX();
    double y = m_data.view().locationY();

    m_data.view().translateToSee(x, y, 0.0, 0.0);
}

void Backend::setLocationAvailable(bool enabled)
{
#if defined(FORDIL_DEBUG_BACKEND)
    qDebug() << "setLocationAvailable" << (enabled ? "Y" : "N");
#endif

    if (isLocationAvailable() != enabled)
    {
        m_data.view().setLocationAvailable(enabled);
        emit isLocationAvailableChanged();
    }
}

bool Backend::isLocationAvailable()
{
    bool result = m_data.view().isLocationAvailable();
    return result;
}

void Backend::setLocation(double latitude, double longitude)
{
    m_data.view().setLocation(latitude, longitude);

    double x = m_data.site().longitudeToX(longitude);
    double y = m_data.site().latitudeToY(latitude);
    m_data.view().setLocationXY(x, y);
}

void Backend::setAzimuthAvailable(bool enabled)
{
#if defined(FORDIL_DEBUG_BACKEND)
    qDebug() << "setAzimuthAvailable" << (enabled ? "Y" : "N");
#endif

    m_data.view().setAzimuthAvailable(enabled);
}

void Backend::setAzimuth(double azimuth)
{
    m_data.view().setAzimuth(azimuth);
}

// Selection
bool Backend::click(const QString &mapId, double x, double y, double w, double h)
{
#if defined(FORDIL_DEBUG_BACKEND)
    qDebug() << "click" << mapId << x << "," << y << "in" << w << "," << h;
#endif

    double rx = x / w;
    double ry = y / h;

    if (!m_data.loadedDiscoveryPart())
    {
        m_data.setTreePreselected(m_selectedTree, 0);
        m_data.setSpotPreselected(m_selectedSpot, 0);
    }

    MapPainter::click(m_selectedTree,
                   m_selectedHotspot,
                   m_selectedSpot,
                   m_selectedStop,
                   m_selectedPoint,
                   m_data,
                   rx,
                   ry);

    return m_selectedTree.hasHit() ||
           m_selectedHotspot.hasHit() ||
           m_selectedSpot.hasHit() ||
           m_selectedStop.hasHit();
}

void Backend::resetInteraction()
{
    if (m_selectedTree.hasHit())
    {
        auto treeId = m_selectedTree.treeId();
        if (m_data.trees().find(treeId) != m_data.trees().end())
        {
            m_data.trees().at(treeId).resetSelection();
        }
    }

    m_selectedHotspot.reset();
    m_selectedPoint.reset();
}

void Backend::cancelSelection()
{
    auto treeId = m_selectedTree.treeId();
    if (m_data.trees().find(treeId) != m_data.trees().end())
    {
        m_data.trees().at(treeId).cancelSelection();
    }

    m_selectedTree.reset();
}

void Backend::confirmSelection(bool competitionChecked,
                               bool maturityChecked,
                               bool promoteRegenerationChecked,
                               bool promoteRareSpeciesChecked,
                               bool sanitaryChecked,
                               bool technicalChecked,
                               bool badQualityChecked)
{
    TreeSelectionChoices selectionChoices(competitionChecked,
                                          maturityChecked,
                                          promoteRegenerationChecked,
                                          promoteRareSpeciesChecked,
                                          sanitaryChecked,
                                          technicalChecked,
                                          badQualityChecked);

    m_data.trees().at(m_selectedTree.treeId()).confirmSelection(selectionChoices);
    m_selectedTree.reset();
}

bool Backend::isTreeSelected() const
{
    return m_selectedTree.isSelected();
}

bool Backend::isCompetitionChecked() const
{
    return m_selectedTree.treeSelectionChoices().competitionChecked();
}

bool Backend::isMaturityChecked() const
{
    return m_selectedTree.treeSelectionChoices().maturityChecked();
}

bool Backend::isPromoteRegenerationChecked() const
{
    return m_selectedTree.treeSelectionChoices().promoteRegenerationChecked();
}

bool Backend::isPromoteRareSpeciesChecked() const
{
    return m_selectedTree.treeSelectionChoices().promoteRareSpeciesChecked();
}

bool Backend::isSanitaryChecked() const
{
    return m_selectedTree.treeSelectionChoices().sanitaryChecked();
}

bool Backend::isTechnicalChecked() const
{
    return m_selectedTree.treeSelectionChoices().technicalChecked();
}

bool Backend::isBadQualityChecked() const
{
    return m_selectedTree.treeSelectionChoices().technicalChecked();
}

QString Backend::selectedTreeId() const
{
    return QString::number(m_selectedTree.treeId());
}

QString Backend::selectedTreeSpecies() const
{
    return m_selectedTree.speciesName();
}

QString Backend::selectedTreeDBH() const
{
    return QString::number(m_selectedTree.dbh() * 100.0);
}

QString Backend::selectedTreeGPS() const
{
    return m_selectedTree.gps();
}

double Backend::selectedTreeLatitude() const
{
    return m_selectedTree.latitude();
}

double Backend::selectedTreeLongitude() const
{
    return m_selectedTree.longitude();
}

double Backend::selectedPointLatitude() const
{
    return m_selectedPoint.latitude();
}

double Backend::selectedPointLongitude() const
{
    return m_selectedPoint.longitude();
}

// Chart
void Backend::initCurveOptions()
{
    m_chartOptions.curves.clear();

    m_chartOptions.curves.push_back({"MeyerTypeA",
                                    "mixed forest - type A (Meyer)",
                                    QColor(0, 0, 255), false, 0.055, 41.4});

    m_chartOptions.curves.push_back({"MeyerTypeB",
                                    "mixed forest - type B (Meyer)",
                                    QColor(255, 165, 0), false, 0.06, 56.5});

    m_chartOptions.curves.push_back({"MeyerTypeC",
                                    "mixed forest - type C (Meyer)",
                                    QColor(128, 0, 128), false, 0.065, 71.7});

    m_chartOptions.curves.push_back({"MeyerTypeD",
                                    "mixed forest - type D (Meyer)",
                                    QColor(0, 255, 255), false, 0.07, 86.9});

    m_chartOptions.curves.push_back({"MeyerTypeE",
                                    "mixed forest - type E (Meyer)",
                                    QColor(150, 75, 0), false, 0.075, 102.1});

    m_chartOptions.curves.push_back({"Beech",
                                    "beech forest (Biehl)",
                                    QColor(0, 128, 0), false, 0.055, 160.0});
}

QVariantList Backend::populateCurveOptions()
{
    QVariantList list;

    for (const auto &it : m_chartOptions.curves)
    {
        list << QVariantMap{{"id", it.id},
                            {"text", m_data.text(it.name)},
                            {"checked", it.enabled}};
    }

    return list;
}

Q_INVOKABLE void Backend::setSelectedCurveOptions(const QStringList &options)
{
#if defined(FORDIL_DEBUG_BACKEND)
    qDebug() << "setSelectedCurveOptions" << options;
#endif

    for (auto &itCurve : m_chartOptions.curves)
    {
        itCurve.enabled = false;
    }

    for (const auto &itOption : options)
    {
        for (auto &itCurve : m_chartOptions.curves)
        {
            if (itCurve.id == itOption)
            {
                itCurve.enabled = true;
            }
        }
    }
}

// Typology
void Backend::calculateTypologyLocal()
{
    m_data.calculateTypologyLocal();
}

void Backend::calculateTypologyGlobal()
{
    m_data.calculateTypologyGlobal();
}

void Backend::calculateOverallAssessment()
{
    m_data.calculateOverallAssessment();
}

double Backend::typologyLocalResultsNorthAngle() const
{
    return m_data.typology().North_angle();
}

// Hotspots
QString Backend::hotspotsRankingText()
{
    Result::Valuation hotspots;
    Result::hotspots(hotspots, m_data);

    return hotspots.ranking.text;
}

QString Backend::hotspotsRankingColor()
{
    Result::Valuation hotspots;
    Result::hotspots(hotspots, m_data);

    return hotspots.ranking.color;
}

QVariantList Backend::hotspotsModelData()
{
    return Chart::hotspotsModelData(m_data);
}

QString Backend::hotspotsReasonText()
{
    QString reasonText = m_data.view().hotspotsReasonText();
#if defined(FORDIL_DEBUG_BACKEND)
    qDebug() << "hotspotsReasonText" << reasonText;
#endif
    return reasonText;
}

void Backend::setHotspotsReasonText(const QString &reason)
{
#if defined(FORDIL_DEBUG_BACKEND)
    qDebug() << "setHotspotsReasonText" << reason;
#endif

    TreeSelectionReasonValue reasonEnum = treeSelectionReasonFromString(reason);
    m_data.view().setHotspotsReason(reasonEnum);
    m_data.view().setHotspotsReasonText(reason);
}

QString Backend::hotspotsPedagogicalHeader()
{
    return m_selectedHotspot.pedagogicalHeader();
}

QString Backend::hotspotsPedagogicalContents()
{
    QString str = m_selectedHotspot.pedagogicalContents();
    m_data.formatTextImageSize(str);
    return str;
}

// Travailoscope
QString Backend::travailloscopeQuestionHeader(uint64_t questionId)
{
    return m_data.travailloscopeQuestionHeader(questionId);
}

QVariantList Backend::travailoscopeCheckBoxItems(uint64_t questionId)
{
    return m_data.travailloscopeQuiz().checkBoxItems(m_selectedSpot.spotId(), questionId);
}

void Backend::clearTravailoscopeAnswers()
{
    m_data.clearTravailloscopeAnswers(m_selectedSpot.spotId());
}

void Backend::setTravailloscopeAnswer(uint64_t questionId, const QStringList &selected)
{
#if defined(FORDIL_DEBUG_BACKEND)
    qDebug() << "selected answers for spot" << m_selectedSpot.spotId() << "to questionId" << questionId << ":" << selected;
#endif

    std::set<uint64_t> answer;
    for (const QString &str : selected)
    {
        // Convert to unsigned 64-bit integer and insert into set
        bool ok = false;
        uint64_t number = str.toULongLong(&ok);
        if (ok)
        {
            answer.insert(number);
        }
        else
        {
            // Log or handle invalid number formats
            qWarning() << "Invalid format" << str << "in" << selected;
        }
    }

    m_data.setTravailloscopeAnswer(m_selectedSpot.spotId(), questionId, answer);
}

bool Backend::isTravailloscopeSpotFinished()
{
    return m_data.isTravailloscopeSpotFinished(m_selectedSpot.spotId());
}

int Backend::travailloscopeScore(uint64_t questionId)
{
    return m_data.travailloscopeScore(m_selectedSpot.spotId(), questionId);
}

QString Backend::travailoscopeAnswerText(uint64_t questionId)
{
    return m_data.travailloscopePedagogicalContents(m_selectedSpot.spotId(), questionId);
}

// Travailoscope results
void Backend::travailloscopeCalculateScore()
{
    m_data.travailloscopeCalculateScore();
}

QString Backend::valuationRankingText()
{
    return m_data.travailloscopeScoreText();
}

QString Backend::valuationRankingColor()
{
    return m_data.travailloscopeScoreColor();
}

QVariantList Backend::valuationModelData()
{
    return m_data.travailloscopeScoreModelData();
}

QString Backend::valuationReasonText()
{
    QString reasonText = m_data.view().valuationReasonText();
#if defined(FORDIL_DEBUG_BACKEND)
    qDebug() << "valuationReasonText" << reasonText;
#endif
    return reasonText;
}

void Backend::setValuationReasonText(const QString &reason)
{
#if defined(FORDIL_DEBUG_BACKEND)
    qDebug() << "setValuationReasonText" << reason;
#endif

    m_data.view().setValuationReasonText(reason);
}

QString Backend::valuationPedagogicalHeader()
{
    return m_data.travailloscopePedagogicalHeader(m_selectedSpot.spotId());
}

QString Backend::valuationPedagogicalContents()
{
    return m_data.travailloscopePedagogicalContents(m_selectedSpot.spotId(), 0);
}

// Discovery
void Backend::discoveryResetQuiz()
{
    m_data.discoveryResetQuiz();
}

int Backend::discoveryQuestion()
{
    return m_data.discoveryQuestion();
}

bool Backend::discoveryHasQuestions()
{
    return m_data.discoveryHasQuestions();
}

bool Backend::discoveryHasNextQuestion()
{
    return m_data.discoveryHasNextQuestion();
}

void Backend::discoveryNextQuestion()
{
    m_data.discoveryNextQuestion();
}

QString Backend::discoveryQuestionHeader(uint64_t questionId)
{
    return m_data.discoveryQuestionHeader(questionId);
}

QVariantList Backend::discoveryCheckBoxItems(uint64_t questionId)
{
    return m_data.discoveryCheckBoxItems(questionId);
}

QString Backend::discoveryQuestionText(uint64_t questionId)
{
    return m_data.discoveryQuestionText(questionId);
}

QString Backend::discoveryAnswerHeader(uint64_t questionId)
{
    return m_data.discoveryAnswerHeader(questionId);
}

QString Backend::discoveryAnswerText(uint64_t questionId)
{
    return m_data.discoveryAnswerText(questionId);
}

int Backend::discoveryState()
{
    return m_data.discoveryState();
}

bool Backend::discoveryHasNextState()
{
    return m_data.discoveryHasNextState();
}

void Backend::discoveryNextState()
{
    m_data.discoveryNextState();
}

int Backend::discoveryStop()
{
    return m_data.discoveryStop();
}

QString Backend::discoveryStopMessage()
{
    return m_data.discoveryStopMessage();
}

QString Backend::discoveryHeader()
{
    return m_data.discoveryHeader();
}

QString Backend::discoveryText()
{
    return m_data.discoveryText();
}

void Backend::discoveryGoToTree()
{
    uint64_t treeId = m_data.discoveryTreeId();
    m_data.setTreePreselected(m_selectedTree, treeId);
}
