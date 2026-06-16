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

#ifndef C_DATA_H
#define C_DATA_H

#include <QDateTime>
#include "../3rdparty/QXlsx/header/xlsxdocument.h"
#include "c_type.h"

#define FORDIL_BUNDLE_SITE 1

class Data
{
public:
    Data();

    View &view() { return m_view; }
    const Site &site() const { return m_site; }
    const Typology &typology() const { return m_typology; }
    const std::unordered_map<uint64_t, Species> &species() const { return m_species; }
    const std::map<QString, Species> &speciesSite() const { return m_speciesSite; }
    std::unordered_map<uint64_t, Tree> &trees() { return m_trees; }
    const std::unordered_map<uint64_t, Tree> &trees() const { return m_trees; }
    const std::vector<uint64_t> &treeIdsSortedByDbh() const { return m_treeIdsSortedByDbh; }
    const std::map<uint64_t, Hotspot> hotspots() const { return m_hotspotList; }

    // Text
    void setupWindow(int screenWidth, int screenHeight);
    void loadText();
    QString text(const QString &identifier) const;
    void italicizeWords(QString& html, const QSet<QString>& words) const;
    void italicizeWords(QString& html) const;
    void formatTextImageSize(QString &str,
                             int maxWidth = 0,
                             int autoWidth = 400) const;

    // Exercise
    static void inspectDataFile(const QString &filePath,
                                QString &system,
                                QString &siteName,
                                QString &error);

    static QString validateDataFile(const QString &filePath,
                                    const QString &system,
                                    bool discovery);

    void loadExercise(const QString &filePath,
                      const QString &siteName,
                      const QString &system,
                      bool discovery);

    void clear();

    const QString &loadedExerciseName() const { return m_site.name(); }
    const QString &loadedSystem() const { return m_site.system(); }
    bool loadedDiscoveryPart() const { return m_discoveryPart; }

    bool isExerciseFinished() const { return m_exerciseFinished; }
    void setExerciseFinished(bool value);
    QString elapsedTime() const;

    QString introductionSiteHeader() const;
    QString introductionSiteBody() const;

    // Selection
    bool setTreePreselected(SelectedTree &selectedTree, uint64_t treeId);
    bool setSpotPreselected(SelectedSpot &selectedSpot, uint64_t spotId);

    // Typology
    void calculateTypologyLocal();
    void calculateTypologyGlobal();
    void calculateOverallAssessment();

    // Travailoscope
    const std::unordered_map<uint64_t, Spot> &spots() const { return m_spots; }
    const TravailloscopeQuiz &travailloscopeQuiz() const { return m_travailloscopeQuiz; }

    QString travailloscopeQuestionHeader(uint64_t questionId) const;
    void clearTravailloscopeAnswers(uint64_t spotId);
    void setTravailloscopeAnswer(uint64_t spotId, uint64_t question, const std::set<uint64_t> &answer);
    bool isTravailloscopeSpotFinished(uint64_t spotId) const;
    int travailloscopeScore(uint64_t spotId, uint64_t question) const;
    QString travailloscopePedagogicalHeader(uint64_t spotId) const;
    QString travailloscopePedagogicalContents(uint64_t spotId, uint64_t question) const;
    void travailloscopeCalculateScore();
    QString travailloscopeScoreText() const;
    QString travailloscopeScoreColor() const;
    QVariantList travailloscopeScoreModelData() const;

    // Discovery
    void discoveryResetQuiz();
    int discoveryQuestion();
    bool discoveryHasQuestions();
    bool discoveryHasNextQuestion();
    void discoveryNextQuestion();
    QString discoveryQuestionHeader(uint64_t questionId);
    QVariantList discoveryCheckBoxItems(uint64_t questionId);
    QString discoveryQuestionText(uint64_t questionId);
    QString discoveryAnswerHeader(uint64_t questionId);
    QString discoveryAnswerText(uint64_t questionId);

    int discoveryState();
    bool discoveryHasNextState();
    void discoveryNextState();
    int discoveryStop();
    uint64_t discoveryTreeId();
    QString discoveryStopMessage();
    QString discoveryHeader();
    QString discoveryText();

private:
    View m_view;
    Site m_site;
    Typology m_typology;
    std::unordered_map<uint64_t, Species> m_species;
    std::unordered_map<std::string, uint64_t> m_speciesFullName;
    std::map<QString, Species> m_speciesSite;
    std::unordered_map<uint64_t, EcologicalValue> m_ecologicalValues;
    std::unordered_map<uint64_t, Tree> m_trees;
    std::vector<uint64_t> m_treeIdsSortedByDbh;
    std::map<uint64_t, Hotspot> m_hotspotList;
    std::unordered_map<uint64_t, Spot> m_spots;
    TravailloscopeQuiz m_travailloscopeQuiz;
    DiscoveryQuiz m_discoveryQuiz;
    DiscoveryTour m_discoveryTour;
    QString m_introduction;
    std::unordered_map<QString, QString> m_text;

    bool m_discoveryPart{false};
    int m_discoveryStep{0};
    int m_discoveryState{0};

    bool m_exerciseFinished{false};
    QDateTime m_startTime;
    QDateTime m_endTime;

    int m_screenWidth{300};
    int m_screenHeight{700};

    // Helpers
    void addTree(uint64_t treeId,
                 uint64_t speciesId,
                 double latitude,
                 double longitude,
                 size_t gridId,
                 double dbh,
                 double dbh10,
                 double height,
                 const std::map<QString, double> &volumeQual,
                 double cv,
                 double yGain,
                 const QString &option,
                 double ecovalueGlob,
                 double q4Prosilvalue,
                 bool dead,
                 bool bio,
                 const std::vector<int> &dmh);

    // General site information
    void loadSite(QXlsx::Document &xlsx, const QString &siteName, const QString &system);
    void loadIntroduction(QXlsx::Document &xlsx);

    // Marteloscope
    void loadSpecies(QXlsx::Document &xlsx);
    void loadEcologicalValues(QXlsx::Document &xlsx);
    void loadTrees(QXlsx::Document &xlsx);
    void updateQ4Prosilvalue();
    void updateSiteDim();
    void loadTreeLogs(QXlsx::Document &xlsx);
    void loadTypology(QXlsx::Document &xlsx);
    void loadExperts(QXlsx::Document &xlsx);
    void loadHotspots(QXlsx::Document &xlsx);
    void loadDiscoveryQuiz(QXlsx::Document &xlsx);
    void loadDiscovery(QXlsx::Document &xlsx);

    // Travailloscope
    void loadTravailloscopeSpot(QXlsx::Document &xlsx);
    void updateSiteDimFromSpots();
    void loadTravailloscopeQuestion(QXlsx::Document &xlsx);
    void loadTravailloscopeAnswer(QXlsx::Document &xlsx);

    QString bundledSitePath(const QString &filePath,
                            const QString &siteName,
                            const QString &system);
};

#endif // C_DATA_H
