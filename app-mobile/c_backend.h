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

#ifndef c_BACKEND_H
#define c_BACKEND_H

#include <QApplication>
#include <QTranslator>
#include <QQuickImageProvider>
#include <QVariantList>
#include "../backend/c_data.h"
#include "../backend/c_chart.h"

class Backend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QStringList logLines READ logLines NOTIFY logLinesChanged)

    Q_PROPERTY(bool isDebugBuild READ isDebugBuild  CONSTANT)
    Q_PROPERTY(bool isDebugModeEnabled READ isDebugModeEnabled WRITE setDebugModeEnabled NOTIFY isDebugModeEnabledChanged)

    Q_PROPERTY(bool isExerciseFinished READ isExerciseFinished WRITE setExerciseFinished NOTIFY isExerciseFinishedChanged)
    Q_PROPERTY(bool isLocationAvailable READ isLocationAvailable WRITE setLocationAvailable NOTIFY isLocationAvailableChanged())
    Q_PROPERTY(bool isGpsEnabled READ isGpsEnabled WRITE setGpsEnabled NOTIFY isGpsEnabledChanged)
    Q_PROPERTY(bool isShowGpsCoordinatesEnabled READ isShowGpsCoordinatesEnabled WRITE setShowGpsCoordinatesEnabled NOTIFY isShowGpsCoordinatesEnabledChanged)
    Q_PROPERTY(QString treeDistributionOption READ treeDistributionOption WRITE setTreeDistributionOption NOTIFY treeDistributionOptionChanged)
    Q_PROPERTY(QString typologyLocalResultsSettings READ typologyLocalResultsSettings WRITE setTypologyLocalResultsSettings NOTIFY typologyLocalResultsSettingsChanged)
    Q_PROPERTY(QString typologyGlobalResultsSettings READ typologyGlobalResultsSettings WRITE setTypologyGlobalResultsSettings NOTIFY typologyGlobalResultsSettingsChanged)
    Q_PROPERTY(double typologyLocalResultsNorthAngle READ typologyLocalResultsNorthAngle NOTIFY typologyLocalResultsNorthAngleChanged)

public:
    explicit Backend(QApplication *application, QObject *parent = nullptr);

    // Init
    void init();
    void setupWindow();

    // Log
    QStringList logLines() const { return m_logLines; }
    Q_INVOKABLE void appendLogLine(const QString &level, const QString &message);

    // Settings
    Q_INVOKABLE void setSetting(const QString &key, const QVariant &value);
    Q_INVOKABLE QVariant getSetting(const QString &key, const QVariant &defaultValue = QVariant());

    // Settings Misc
    Q_INVOKABLE bool isDebugBuild() const;
    Q_INVOKABLE bool isDebugModeEnabled();
    Q_INVOKABLE void setDebugModeEnabled(bool enabled);

    Q_INVOKABLE bool isDebugQmlEnabled();

    // Settings GPS
    Q_INVOKABLE bool isGpsEnabled();
    Q_INVOKABLE void setGpsEnabled(bool enabled);

    Q_INVOKABLE bool isShowGpsCoordinatesEnabled();
    Q_INVOKABLE void setShowGpsCoordinatesEnabled(bool enabled);

    // Settings Tree selection
    Q_INVOKABLE double treeScale();
    Q_INVOKABLE void setTreeScale(double scale);

    // Settings Results
    Q_INVOKABLE bool isSpeciesInTreeStatisticsEnabled();
    Q_INVOKABLE void setSpeciesInTreeStatisticsEnabled(bool enabled);

    Q_INVOKABLE bool isRoundValuesEnabled();
    Q_INVOKABLE void setRoundValuesEnabled(bool enabled);

    Q_INVOKABLE QString treeDistributionOption();
    Q_INVOKABLE void setTreeDistributionOption(const QString &option);

    Q_INVOKABLE QString typologyLocalResultsSettings();
    Q_INVOKABLE void setTypologyLocalResultsSettings(const QString &option);

    Q_INVOKABLE QString typologyGlobalResultsSettings();
    Q_INVOKABLE void setTypologyGlobalResultsSettings(const QString &option);

    Q_INVOKABLE void setShowMapTreeSpeciesLegend();

    // Language
    Q_INVOKABLE QString systemLanguageCode() const;
    Q_INVOKABLE void setLanguage();

    Q_INVOKABLE QString languageCode();
    Q_INVOKABLE void setLanguageByCode(const QString &code);

    Q_INVOKABLE QString text(const QString &identifier) const;

    // Exercise
    Q_INVOKABLE QString storeFile(const QString &path);

    Q_INVOKABLE QVariantList siteNamesModelData(const QString &system, bool discovery);

    Q_INVOKABLE void loadExercise(const QString &filePath,
                                  const QString &siteName,
                                  const QString &system,
                                  bool discovery,
                                  bool reload);

    Q_INVOKABLE QString loadedExerciseName() const;
    Q_INVOKABLE QString loadedSystem() const;
    Q_INVOKABLE bool loadedDiscoveryPart() const;

    bool isExerciseFinished() const;
    Q_INVOKABLE void setExerciseFinished(bool value);
    Q_INVOKABLE QString elapsedExerciseTime() const;

    // Introduction screens
    Q_INVOKABLE bool isIntroductionAppVisible() const;
    Q_INVOKABLE void setIntroductionAppVisible(bool visible);

    Q_INVOKABLE bool isIntroductionSiteVisible() const;
    Q_INVOKABLE void setIntroductionSiteVisible(bool visible);
    Q_INVOKABLE QString introductionSiteHeader() const;
    Q_INVOKABLE QString introductionSiteBody() const;

    Q_INVOKABLE bool isIntroductionResultsVisible() const;
    Q_INVOKABLE void setIntroductionResultsVisible(bool visible);

    Q_INVOKABLE bool isIntroductionGlobalResultsVisible() const;
    Q_INVOKABLE void setIntroductionGlobalResultsVisible(bool visible);

    Q_INVOKABLE bool isIntroductionAdvancedResultsVisible() const;
    Q_INVOKABLE void setIntroductionAdvancedResultsVisible(bool visible);

    // Image provider
    class ImageProvider : public QQuickImageProvider
    {
    public:
        ImageProvider(Backend *backend);
        QImage requestImage(const QString &imageId, QSize *size, const QSize &requestedSize) override;
    private:
        Backend *m_backend;
    };

    ImageProvider *getImageProvider() { return &m_imageProvider; }
    Q_INVOKABLE bool isRedrawAllowed();
    QImage requestImage(const QString &imageId, QSize *size, const QSize &requestedSize);
    Q_INVOKABLE double imageHeight(const QString &imageId, double width);
    Q_INVOKABLE void updateChartOptions();

    // PDF
    Q_INVOKABLE QString copyResourceToUrl(const QString &resourcePath, const QUrl &targetUrl);
    Q_INVOKABLE QString glossaryPdfFileName();
    Q_INVOKABLE QString copyGlossaryToUrl(const QUrl &targetUrl);
    Q_INVOKABLE QString exportTravailloscopeQuizToPdf(const QUrl &targetUrl);
    Q_INVOKABLE QString exportMarteloscopeResultsToPdf(const QUrl &targetUrl);

    // Site map
    Q_INVOKABLE double zoom(const QString &mapId);
    Q_INVOKABLE void setZoom(const QString &mapId, double value, double pinchScale, double x, double y, double w, double h);
    Q_INVOKABLE void zoomIn(const QString &mapId);
    Q_INVOKABLE void zoomOut(const QString &mapId);
    Q_INVOKABLE void resetZoom(const QString &mapId);

    Q_INVOKABLE void translate(const QString &mapId, double dx, double dy);
    Q_INVOKABLE void translateToSelection(const QString &mapId);
    Q_INVOKABLE bool translateToId(const QString &text);
    Q_INVOKABLE void resetTranslation(const QString &mapId);
    Q_INVOKABLE void refocus(const QString &mapId);

    Q_INVOKABLE void setLocationAvailable(bool enabled);
    Q_INVOKABLE bool isLocationAvailable();
    Q_INVOKABLE void setLocation(double latitude, double longitude);

    Q_INVOKABLE void setAzimuthAvailable(bool enabled);
    Q_INVOKABLE void setAzimuth(double azimuth);

    // Selection
    Q_INVOKABLE bool click(const QString &mapId, double x, double y, double w, double h);
    Q_INVOKABLE void resetInteraction();
    Q_INVOKABLE void cancelSelection();
    Q_INVOKABLE void confirmSelection(bool competitionChecked,
                                      bool maturityChecked,
                                      bool promoteRegenerationChecked,
                                      bool promoteRareSpeciesChecked,
                                      bool sanitaryChecked,
                                      bool technicalChecked,
                                      bool badQualityChecked);

    Q_INVOKABLE bool isTreeSelected() const;
    Q_INVOKABLE bool isCompetitionChecked() const;
    Q_INVOKABLE bool isMaturityChecked() const;
    Q_INVOKABLE bool isPromoteRegenerationChecked() const;
    Q_INVOKABLE bool isPromoteRareSpeciesChecked() const;
    Q_INVOKABLE bool isSanitaryChecked() const;
    Q_INVOKABLE bool isTechnicalChecked() const;
    Q_INVOKABLE bool isBadQualityChecked() const;

    Q_INVOKABLE QString selectedTreeId() const;
    Q_INVOKABLE QString selectedTreeSpecies() const;
    Q_INVOKABLE QString selectedTreeDBH() const;
    Q_INVOKABLE QString selectedTreeGPS() const;
    Q_INVOKABLE double selectedTreeLatitude() const;
    Q_INVOKABLE double selectedTreeLongitude() const;

    Q_INVOKABLE double selectedPointLatitude() const;
    Q_INVOKABLE double selectedPointLongitude() const;

    // Chart
    Q_INVOKABLE void initCurveOptions();
    Q_INVOKABLE QVariantList populateCurveOptions();
    Q_INVOKABLE void setSelectedCurveOptions(const QStringList &options);

    // Typology
    Q_INVOKABLE void calculateTypologyLocal();
    Q_INVOKABLE void calculateTypologyGlobal();
    Q_INVOKABLE void calculateOverallAssessment();
    Q_INVOKABLE double typologyLocalResultsNorthAngle() const;

    // Hotspots
    Q_INVOKABLE QString hotspotsRankingText();
    Q_INVOKABLE QString hotspotsRankingColor();
    Q_INVOKABLE QVariantList hotspotsModelData();

    Q_INVOKABLE QString hotspotsReasonText();
    Q_INVOKABLE void setHotspotsReasonText(const QString &reason);

    Q_INVOKABLE QString hotspotsPedagogicalHeader();
    Q_INVOKABLE QString hotspotsPedagogicalContents();

    // Travailoscope
    Q_INVOKABLE QString travailloscopeQuestionHeader(uint64_t questionId);
    Q_INVOKABLE QVariantList travailoscopeCheckBoxItems(uint64_t questionId);
    Q_INVOKABLE void clearTravailoscopeAnswers();
    Q_INVOKABLE void setTravailloscopeAnswer(uint64_t questionId, const QStringList &selected);
    Q_INVOKABLE bool isTravailloscopeSpotFinished();
    Q_INVOKABLE int travailloscopeScore(uint64_t questionId);
    Q_INVOKABLE QString travailoscopeAnswerText(uint64_t questionId);

    // Travailoscope results
    Q_INVOKABLE void travailloscopeCalculateScore();
    Q_INVOKABLE QString valuationRankingText();
    Q_INVOKABLE QString valuationRankingColor();
    Q_INVOKABLE QVariantList valuationModelData();

    Q_INVOKABLE QString valuationReasonText();
    Q_INVOKABLE void setValuationReasonText(const QString &reason);

    Q_INVOKABLE QString valuationPedagogicalHeader();
    Q_INVOKABLE QString valuationPedagogicalContents();

    // Discovery
    Q_INVOKABLE void discoveryResetQuiz();
    Q_INVOKABLE int discoveryQuestion();
    Q_INVOKABLE bool discoveryHasQuestions();
    Q_INVOKABLE bool discoveryHasNextQuestion();
    Q_INVOKABLE void discoveryNextQuestion();
    Q_INVOKABLE QString discoveryQuestionHeader(uint64_t questionId);
    Q_INVOKABLE QVariantList discoveryCheckBoxItems(uint64_t questionId);
    Q_INVOKABLE QString discoveryQuestionText(uint64_t questionId);
    Q_INVOKABLE QString discoveryAnswerHeader(uint64_t questionId);
    Q_INVOKABLE QString discoveryAnswerText(uint64_t questionId);

    Q_INVOKABLE int discoveryState();
    Q_INVOKABLE bool discoveryHasNextState();
    Q_INVOKABLE void discoveryNextState();
    Q_INVOKABLE int discoveryStop();
    Q_INVOKABLE QString discoveryStopMessage();
    Q_INVOKABLE QString discoveryHeader();
    Q_INVOKABLE QString discoveryText();
    Q_INVOKABLE void discoveryGoToTree();

signals:
    void logLinesChanged();
    void isExerciseFinishedChanged();
    void isLocationAvailableChanged();
    void isGpsEnabledChanged();
    void isShowGpsCoordinatesEnabledChanged();
    void isDebugModeEnabledChanged();
    void treeDistributionOptionChanged();
    void typologyLocalResultsSettingsChanged();
    void typologyGlobalResultsSettingsChanged();
    void typologyLocalResultsNorthAngleChanged();

private:
    // Application
    QApplication *m_application;

    // Log
    QStringList m_logLines;

    // Language
    QTranslator m_translator;

    // Image
    ImageProvider m_imageProvider;
    QImage m_image;
    QImage m_emptyImage;
    QDateTime m_lastDrawTime;

    // Data
    Data m_data;
    Chart::Options m_chartOptions;

    // Data selection
    SelectedTree m_selectedTree;
    SelectedHotspot m_selectedHotspot;
    SelectedSpot m_selectedSpot;
    SelectedStop m_selectedStop;
    SelectedPoint m_selectedPoint;

    // Introduction screens
    bool m_showIntroductionApp{true};
    bool m_showIntroductionSite{false};
    bool m_showIntroductionResults{false};
    bool m_showIntroductionGlobalResults{false};
    bool m_showIntroductionAdvancedResults{false};
};

#endif // c_BACKEND_H
