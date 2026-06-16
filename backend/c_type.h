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

#ifndef C_TYPE_H
#define C_TYPE_H

#include <vector>
#include <set>
#include <string>
#include <stdexcept>
#include <unordered_map>
#include <map>
#include <cstdint>
#include <memory>
#include <QString>
#include <QColor>

// #define FORDIL_DEBUG_IMAGE 1

enum class Component : size_t
{
    Before,
    Cut,
    After,
    End
};

template <typename E>
constexpr auto toIndex(E e) noexcept {
    return static_cast<std::underlying_type_t<E>>(e);
}

class Translation
{
public:
   std::map<QString,QString> language;
};

class Core
{
public:
    void clear();
    void setLanguageByCode(const QString &code);
    void insertTranslation(const QString &source, const QString &translation, const QString &languageCode);
    const QString &translate(const QString &text) const;

private:
    QString m_languageCode;
    std::map<QString,Translation> m_translations; // source:translation
};

Core &core();

class Species
{
public:
    Species();
    Species(uint64_t speciesId,
            const QString &fullName,
            const QColor &color,
            const std::map<QString, double> &price);

    uint64_t speciesId() const { return m_speciesId; }
    const QString &fullName() const { return m_fullName; }
    const QColor &color() const { return m_color; }

    double price(const QString &qual) const;
    double cv(const QString &qual, double volume) const;

private:
    uint64_t m_speciesId{0};
    QString m_fullName;
    QColor m_color;
    std::map<QString, double> m_price;
};

enum class SelectionStatus : size_t
{
    None,
    Highlighted,
    Cross,
    Selected
};

enum class TreeSelectionReasonValue : size_t
{
    None,
    Competition,
    Maturity,
    PromoteRegeneration,
    PromoteRareSpecies,
    Sanitary,
    Technical,
    BadQuality,
    Siblings,
    Biodiversity,
    HabitatTree,
    DeadTree,
    End
};

class TreeSelectionReason
{
public:
    TreeSelectionReasonValue value;
    QString label;
    QColor color;
};

extern TreeSelectionReasonValue treeSelectionReasonFromString(const QString &str);
extern QString treeSelectionReasonToString(TreeSelectionReasonValue reason);
extern QString treeSelectionReasonToStringTr(TreeSelectionReasonValue reason);
extern QColor treeSelectionReasonColor(TreeSelectionReasonValue reason);

class TreeSelectionChoices
{
public:
    TreeSelectionChoices();
    TreeSelectionChoices(bool competitionChecked,
                         bool maturityChecked,
                         bool promoteRegenerationChecked,
                         bool promoteRareSpeciesChecked,
                         bool sanitaryChecked,
                         bool technicalChecked,
                         bool badQualityChecked);

    bool competitionChecked() const { return m_competitionChecked; }
    bool maturityChecked() const { return m_maturityChecked; }
    bool promoteRegenerationChecked() const { return m_promoteRegenerationChecked; }
    bool promoteRareSpeciesChecked() const { return m_promoteRareSpeciesChecked; }
    bool sanitaryChecked() const { return m_sanitaryChecked; }
    bool technicalChecked() const { return m_technicalChecked; }
    bool badQualityChecked() const { return m_badQualityChecked; }

    bool checked(TreeSelectionReasonValue cuttingChoice) const;

    bool isSelected() const;

    void reset();

private:
    bool m_competitionChecked{false};
    bool m_maturityChecked{false};
    bool m_promoteRegenerationChecked{false};
    bool m_promoteRareSpeciesChecked{false};
    bool m_sanitaryChecked{false};
    bool m_technicalChecked{false};
    bool m_badQualityChecked{false};
};

class TreeLog
{
public:
    TreeLog();
    TreeLog(uint64_t treeId,
            const QString &qual,
            double vol,
            double cv,
            double yGain);

    uint64_t treeId() const { return m_treeId; }
    const QString &qual() const { return m_qual; }
    double vol() const { return m_vol; }
    double cv() const { return m_cv; }
    double yGain() const { return m_yGain; }

    void add(const TreeLog &other);

private:
    uint64_t m_treeId{0};
    QString m_qual;
    double m_vol{0.0};
    double m_cv{0.0};
    double m_yGain{0.0};
};

class EcologicalValue
{
public:
    EcologicalValue();
    EcologicalValue(uint64_t ecologicalValueId,
                    const QString &theme,
                    const QString &description,
                    double nat,
                    double chiro,
                    double avri,
                    double ento,
                    double global);

    uint64_t ecologicalValueId() const { return m_ecologicalValueId; }
    const QString &theme() const { return m_theme; }
    const QString &description() const { return m_description; }
    double nat() const { return m_nat; }
    double chiro() const { return m_chiro; }
    double avri() const { return m_avri; }
    double ento() const { return m_ento; }
    double global() const { return m_global; }

private:
    uint64_t m_ecologicalValueId{0};
    QString m_theme;
    QString m_description;
    double m_nat{0.0};
    double m_chiro{0.0};
    double m_avri{0.0};
    double m_ento{0.0};
    double m_global{0.0};
};

class EconomicValue
{
public:
    double CV{0.0};
    double Gain{0.0};
    double PoTValue{0.0};
};

class EconomicValues
{
public:
    EconomicValue before;
    EconomicValue after;
    EconomicValue cut;
};

class Tree
{
public:
    Tree();
    Tree(uint64_t treeId,
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

    uint64_t treeId() const { return m_treeId; }
    uint64_t speciesId() const { return m_speciesId; }
    size_t gridId() const { return m_gridId; }

    double dbh() const { return m_dbh; }
    double dbh10() const { return m_dbh10; }
    double gha() const;

    // double height() const { return m_height; }

    void setPosition(double x, double y);
    double x() const { return m_x; }
    double y() const { return m_y; }
    double latitude() const { return m_latitude; }
    double longitude() const { return m_longitude; }

    const QString &option() const { return m_option; }

    double ecovalueGlob() const { return m_ecovalueGlob; }
    void setQ4Prosilvalue(double q4Prosilvalue);
    double q4Prosilvalue() const { return m_q4Prosilvalue; }
    bool isDead() const { return m_dead; }
    bool isBio() const { return m_bio; }
    const std::vector<int> &dmh() const { return m_dmh; }

    bool insertTreeLog(const TreeLog &treeLog);
    double vol(const QString &qual = "") const;
    double cv() const;
    double yGain() const;

    SelectionStatus selectionStatus() const { return m_selectionStatus; }
    void setSelectionStatus(SelectionStatus status) { m_selectionStatus = status; }

    bool isSelected() const { return m_selected; }
    const TreeSelectionChoices &treeSelectionChoices() const { return m_treeSelectionChoices; }

    void resetSelection();
    void cancelSelection();
    void confirmSelection(const TreeSelectionChoices &treeSelectionChoices);

private:
    uint64_t m_treeId{0};
    uint64_t m_speciesId{0};

    double m_x{0.0};
    double m_y{0.0};
    double m_latitude{0.0};
    double m_longitude{0.0};
    size_t m_gridId{0};

    double m_dbh{0.0};
    double m_dbh10{0.0};
    double m_height{0.0};

    std::map<QString, double> m_volumeQual;
    double m_cv;
    double m_yGain;

    QString m_option;

    double m_ecovalueGlob{0.0};
    double m_q4Prosilvalue{0.0};
    bool m_dead{false};
    bool m_bio{false};
    std::vector<int> m_dmh;

    SelectionStatus m_selectionStatus{SelectionStatus::None};
    bool m_selected{false};

    TreeSelectionChoices m_treeSelectionChoices;

    std::map<QString, TreeLog> m_treeLog;
};

class HotspotTree
{
public:
    HotspotTree();
    HotspotTree(uint64_t hotspotId,
                uint64_t treeId,
                const std::vector<QString> &cutOptions,
                TreeSelectionReasonValue reason,
                double latitude,
                double longitude,
                double radius,
                const QString &drawings,
                const QString &pedagogicalContents);

    uint64_t hotspotId() const { return m_hotspotId; }
    uint64_t treeId() const { return m_treeId; }
    const std::vector<QString> &cutOptions() const { return m_cutOptions; }
    bool hasCutOption(size_t idx) const;
    bool cutOption(size_t idx) const;
    TreeSelectionReasonValue reason() const { return m_reason; }
    double latitude() const { return m_latitude; }
    double longitude() const { return m_longitude; }
    double radius() const { return m_radius; }
    const QString &drawings() const { return m_drawings; }
    const QString &pedagogicalContents() const { return m_pedagogicalContents; }

private:
    uint64_t m_hotspotId{0};
    uint64_t m_treeId{0};
    std::vector<QString> m_cutOptions;
    TreeSelectionReasonValue m_reason{TreeSelectionReasonValue::None};
    double m_latitude{360.0};
    double m_longitude{360.0};
    double m_radius{0.0};
    QString m_drawings;
    QString m_pedagogicalContents;
};

class Hotspot
{
public:
    Hotspot();
    Hotspot(uint64_t hotspotId);

    uint64_t hotspotId() const { return m_hotspotId; }

    TreeSelectionReasonValue reason() const { return m_reason; }
    double latitude() const { return m_latitude; }
    double longitude() const { return m_longitude; }
    double radius() const { return m_radius; }
    const QString &drawings() const { return m_drawings; }
    const QString &pedagogicalContents() const { return m_pedagogicalContents; }

    void push_back(const HotspotTree &hotspotTree);

    size_t size() const { return m_hotspotTreeList.size(); }
    const HotspotTree &at(size_t index) const { return m_hotspotTreeList.at(index); }
    const auto begin() const { return m_hotspotTreeList.begin(); }
    const auto end() const { return m_hotspotTreeList.end(); }

    std::map<uint64_t, bool> cutResults(const std::unordered_map<uint64_t, Tree> &trees) const;

private:
    uint64_t m_hotspotId{0};
    TreeSelectionReasonValue m_reason{TreeSelectionReasonValue::None};
    double m_latitude{360.0};
    double m_longitude{360.0};
    double m_radius{0.0};
    QString m_drawings;
    QString m_pedagogicalContents;

    std::vector<HotspotTree> m_hotspotTreeList;
};

class TypologyGrid
{
public:
    size_t gridID{0};

    double DBH_lw{0.0};
    double DBH_mw{0.0};
    double DBH_bw{0.0};
    double DBH_vbw{0.0};
    double DBH_bwvbw{0.0};
    int Typo_stru_loc{0};
    int Typo_gha_loc{0};
    QColor color;
    QString label;

    double DBH_lw_10{0.0};
    double DBH_mw_10{0.0};
    double DBH_bw_10{0.0};
    double DBH_vbw_10{0.0};
    double DBH_bwvbw_10{0.0};
    int Typo_stru10_loc{0};

    double Gha_loc{0.0};

    static int struLoc(double lw,
                       double mw,
                       double bw,
                       double vbw,
                       double bwvbw);

    static int ghaLoc(double gha);
};

class TypologyLocal
{
public:
    std::vector<TypologyGrid> grid;

    double DBH_lw{0.0};
    double DBH_mw{0.0};
    double DBH_bwvbw{0.0};

    double DBH_lw_10{0.0};
    double DBH_bwvbw_10{0.0};

    double Gha_loc{0.0};

    void update();
};

class TypologyGlobal
{
public:
    double Struc_n0_X{0.0};
    double Struc_n0_Y{0.0};
    double Struc_n10_X{0.0};
    double Struc_n10_Y{0.0};
    double Gha_n0{0.0};
};

class Typology
{
public:
    void setGrid(double Sha_grid,
                 double Sha_tot,
                 size_t Nb_grid,
                 double Scale_y,
                 double North_angle);

    double Sha_grid() const { return m_Sha_grid; }
    double Sha_tot() const { return m_Sha_tot; }
    size_t Nb_grid() const { return m_Nb_grid; }
    double Scale_y() const { return m_Scale_y; }
    double North_angle() const { return m_North_angle; }

    void setWoodSize(double lw_from,
                     double mw_from,
                     double bw_from,
                     double vbw_from,
                     double bwvbw_from);
    
    void setCutTreeIdsExpert(const std::set<uint64_t> &cutTreeIds);
    const std::set<uint64_t> &cutTreeIdsExpert() const { return m_cutTreeIdsExpert; }

    // Local
    void calculateLocal(const std::unordered_map<uint64_t, Tree> &trees);

    const TypologyLocal &localInit() const { return m_localInit; }
    const TypologyLocal &localExpert() const { return m_localExpert; }
    const TypologyLocal &localUser() const { return m_localUser; }

    // Global
    void calculateGlobal();

    const TypologyGlobal &globalNature() const { return m_globalNature; }
    const TypologyGlobal &globalExpert() const { return m_globalExpert; }
    const TypologyGlobal &globalUser() const { return m_globalUser; }

    // Overall Assessment
    void calculateOverallAssessment();

private:
    double m_Sha_grid{0.0};
    double m_Sha_tot{0.0};
    size_t m_Nb_grid{0};
    double m_Scale_y{0.0};
    double m_North_angle{0.0};

    double m_lw_from{0.0};
    double m_mw_from{0.0};
    double m_bw_from{0.0};
    double m_vbw_from{0.0};
    double m_bwvbw_from{0.0};

    std::set<uint64_t> m_cutTreeIdsExpert;

    TypologyLocal m_localInit;
    TypologyLocal m_localExpert;
    TypologyLocal m_localUser;

    TypologyGlobal m_globalNature;
    TypologyGlobal m_globalExpert;
    TypologyGlobal m_globalUser;

    void calculateGrids(TypologyLocal &typology,
                        const std::unordered_map<uint64_t, Tree> &trees,
                        const std::set<uint64_t> &cutTreeIds);
    void calculateGrid(TypologyGrid &grid,
                       const std::unordered_map<uint64_t, Tree> &trees,
                       const std::set<uint64_t> &cutTreeIds);
    void updateGrid(TypologyGrid &grid);

    void calculateGlobal(TypologyGlobal &typologyGlobal, const TypologyLocal &typologyLocal);
};

class Spot
{
public:
    Spot();
    Spot(uint64_t spotId,
         double latitude,
         double longitude,
         double radius);

    uint64_t spotId() const { return m_spotId; }
    double x() const { return m_x; }
    double y() const { return m_y; }
    double latitude() const { return m_latitude; }
    double longitude() const { return m_longitude; }
    double radius() const { return m_radius; }

    void setPosition(double x, double y);

    SelectionStatus selectionStatus() const { return m_selectionStatus; }
    void setSelectionStatus(SelectionStatus status) { m_selectionStatus = status; }

private:
    uint64_t m_spotId{0};
    double m_x{0.0};
    double m_y{0.0};
    double m_latitude{0.0};
    double m_longitude{0.0};
    double m_radius{0.0};

    SelectionStatus m_selectionStatus{SelectionStatus::None};
};

class TravailloscopeAnswer
{
public:
    TravailloscopeAnswer();

    TravailloscopeAnswer(uint64_t questionId,
                         uint64_t answerId,
                         const QString &answerText,
                         const QString &explanationText);

    uint64_t questionId() const { return m_questionId; }
    uint64_t answerId() const { return m_answerId; }
    const QString &answerText() const { return m_answerText; }
    const QString &explanationText() const { return m_explanationText; }

private:
    uint64_t m_questionId{0};
    uint64_t m_answerId{0};
    QString m_answerText;
    QString m_explanationText;
};

class TravailloscopeSpotAnswer
{
public:
    TravailloscopeSpotAnswer();

    TravailloscopeSpotAnswer(uint64_t spotId,
                             uint64_t questionId,
                             uint64_t answerId,
                             const QString &answerText,
                             const QString &explanationText,
                             bool correct);

    uint64_t spotId() const { return m_spotId; }
    uint64_t questionId() const { return m_questionId; }
    uint64_t answerId() const { return m_answerId; }
    const QString &answerText() const { return m_answerText; }
    const QString &explanationText() const { return m_explanationText; }
    bool isCorrect() const { return m_correct; }

    void updateAnswerId(uint64_t answerId) { m_answerId = answerId; }

private:
    uint64_t m_spotId{0};
    uint64_t m_questionId{0};
    uint64_t m_answerId{0};
    QString m_answerText;
    QString m_explanationText;
    bool m_correct{false};
};

class TravailloscopeSpot
{
public:
    TravailloscopeSpot();
    TravailloscopeSpot(uint64_t spotId);

    uint64_t spotId() const { return m_spotId; }

    void clearAnswers();
    void setAnswer(uint64_t questionId, const std::set<uint64_t> &answer);
    const std::map<uint64_t, std::set<uint64_t>> &answers() const { return m_answers; }

    bool isFinished() const { return m_finished; }
    void setFinished(bool b) { m_finished = b; }

    int score() const { return m_score; }
    void setScore(int score) { m_score = score; }

private:
    uint64_t m_spotId{0};
    std::map<uint64_t, std::set<uint64_t>> m_answers;
    bool m_finished{false};
    int m_score{0};
};

class TravailloscopeQuiz
{
public:
    void insert(uint64_t questionId, const QString &questionHeader);
    void insert(const TravailloscopeAnswer &answer);
    void insert(const TravailloscopeSpotAnswer &answer);
    void insert(const TravailloscopeSpot &spot);
    void shuffle();
    void clear();

    void clearAnswers(uint64_t spotId);
    void setAnswer(uint64_t spotId, uint64_t questionId, const std::set<uint64_t> &answer);
    bool isFinished(uint64_t spotId) const;

    bool hasQuestion(uint64_t spotId, uint64_t questionId) const;
    int calculateScore(uint64_t spotId, uint64_t questionId) const;
    int calculateScore(uint64_t spotId) const;
    void calculateScore();

    int score(uint64_t spotId) const; // 0, 1 (0.5), 2 (1)
    static QString scoreColor(int score);
    static QString scoreText(int score, size_t max);
    QString totalScoreText() const;
    QString totalScoreColor() const;
    QVariantList scoreModelData() const;
    QString scoreTable() const;

    std::vector<uint64_t> spotIdList() const;
    QString spotNo(uint64_t spotId) const;
    QString questionHeader(uint64_t questionId) const;
    std::map<uint64_t, QString> answerText(uint64_t spotId, uint64_t questionId) const;
    QVariantList checkBoxItems(uint64_t spotId, uint64_t questionId) const;
    QString answerText(uint64_t spotId, uint64_t questionId, uint64_t answerId, bool &correct) const;
    QString pedagogicalContentYourAnswers(uint64_t spotId, uint64_t questionId) const;
    QString pedagogicalContent(uint64_t spotId, uint64_t questionId) const;
    QString pedagogicalContents(uint64_t spotId, uint64_t questionId) const;

private:
    std::map<uint64_t, QString> m_questions;
    std::map<uint64_t, std::vector<TravailloscopeAnswer>> m_answers;
    std::map<uint64_t, std::vector<TravailloscopeSpotAnswer>> m_spotAnswers;
    std::map<uint64_t, TravailloscopeSpot> m_spots;
    int m_totalScore{0};

    void shuffleById(std::vector<TravailloscopeSpotAnswer> &list, uint64_t questionId);
};

class DiscoveryState
{
public:
    uint64_t m_stateId{0};
    uint64_t m_stopId{0};
    uint64_t m_treeId{0};
    QString m_treeString;
    QString m_header;
    QString m_text;
};

class DiscoveryTour
{
public:
    void clear();

    void insert(uint64_t stateId,
                uint64_t stopId,
                uint64_t treeId,
                const QString &treeString,
                const QString &header,
                const QString &text);

    size_t size() const { return m_states.size(); }

    int stop(uint64_t stateId) const;
    uint64_t treeId(uint64_t stateId) const;

    QString message(uint64_t stateId) const;
    QString header(uint64_t stateId) const;
    QString text(uint64_t stateId) const;

private:
    std::map<uint64_t, DiscoveryState> m_states;
};

class DiscoveryOption
{
public:
    uint64_t m_optionId{0};
    QString m_imageName;
    QString m_text;
};

class DiscoveryQuestion
{
public:
    QString m_question;
    QString m_questionText;
    std::vector<DiscoveryOption> m_options;
    DiscoveryOption m_answer;
};

class DiscoveryQuiz
{
public:
    void clear();

    void insert(uint64_t questionId,
                uint64_t optionId,
                const QString &type,
                const QString &imageName,
                const QString &text);
    
    size_t size() const { return m_questions.size(); }

    bool hasQuestion(uint64_t questionId) const { return m_questions.count(questionId) > 0; }
    const DiscoveryQuestion &question(uint64_t questionId) const { return m_questions.at(questionId); }

    QString questionHeader(uint64_t questionId) const;
    QString questionText(uint64_t questionId) const;
    QString answerHeader(uint64_t questionId) const;
    QString answerText(uint64_t questionId) const;

private:
    std::map<uint64_t, DiscoveryQuestion> m_questions;
};

class ResultExplanations
{
public:
    void clear();

    void insert(const QString &result,
                const QString &explanation);
    
    QString explanation(const QString &result) const;

private:
    std::map<QString, QString> m_explanations;
};

class SelectedTree
{
public:
    SelectedTree();

    bool hasHit() const { return m_hit; }
    bool isSelected() const { return m_selected; }

    uint64_t treeId() const { return m_treeId; }
    double x() const { return m_x; }
    double y() const { return m_y; }
    double dbh() const { return m_dbh; }
    const QString &speciesName() const { return m_speciesName; }
    const QString &gps() const { return m_gps; }
    double latitude() const { return m_latitude; }
    double longitude() const { return m_longitude; }

    const TreeSelectionChoices &treeSelectionChoices() const { return m_treeSelectionChoices; }

    void pick(const Tree &tree, const Species &species);
    void reset();

private:
    bool m_hit{false};
    bool m_selected{false};

    uint64_t m_treeId{0};
    double m_x{0.0};
    double m_y{0.0};
    double m_dbh{0.0};
    QString m_speciesName;
    QString m_gps;
    double m_latitude{0.0};
    double m_longitude{0.0};

    TreeSelectionChoices m_treeSelectionChoices;
};

class SelectedHotspot
{
public:
    SelectedHotspot();

    bool hasHit() const { return m_hit; }
    uint64_t hotspotId() const { return m_hotspotId; }

    const QString &pedagogicalHeader() const { return m_pedagogicalHeader; }
    const QString &pedagogicalContents() const { return m_pedagogicalContents; }

    void pick(const Hotspot &hotspot);
    void reset();

private:
    bool m_hit{false};
    uint64_t m_hotspotId{0};
    QString m_pedagogicalHeader;
    QString m_pedagogicalContents;
};

class SelectedSpot
{
public:
    SelectedSpot();

    bool hasHit() const { return m_hit; }

    uint64_t spotId() const { return m_spotId; }
    double x() const { return m_x; }
    double y() const { return m_y; }
    double radius() const { return m_radius; }
    const QString &mainSpecies() const { return m_mainSpecies; }

    void pick(const Spot &spot);
    void reset();

private:
    bool m_hit{false};
    uint64_t m_spotId{0};
    double m_x{0.0};
    double m_y{0.0};
    double m_radius{0.0};
    QString m_mainSpecies;
};

class SelectedStop
{
public:
    bool hasHit() const { return m_hit; }
    uint64_t stateId() const { return m_stateId; }

    void pick(uint64_t stateId);
    void reset();

private:
    bool m_hit{false};
    uint64_t m_stateId{0};
};

class SelectedPoint
{
public:
    SelectedPoint();

    double latitude() const { return m_latitude; }
    double longitude() const { return m_longitude; }

    void pick(double latitude, double longitude);
    void reset();

private:
    double m_latitude{0.0};
    double m_longitude{0.0};
};

class Site
{
public:
    Site();
    Site(const QString &name,
         const QString &system,
         double width,
         double height,
         double ha,
         double northAngle);

    const QString &name() const { return m_name; }
    const QString &system() const { return m_system; }

    // Dimension in meters
    double width() const { return m_width; }
    double height() const { return m_height; }
    void setDimension(double width, double height);

    // Area
    double ha() const { return m_ha; }
    double northAngle() const { return m_northAngle; }

    // Location
    void setLocation(double latitudeMin,
                     double longitudeMin,
                     double latitudeMax,
                     double longitudeMax,
                     double latitudeDegreesPerMeter,
                     double longitudeDegreesPerMeter);

    double latitudeMin() const { return m_latitudeMin; }
    double longitudeMin() const { return m_longitudeMin; }

    double latitudeMax() const { return m_latitudeMax; }
    double longitudeMax() const { return m_longitudeMax; }

    double latitudeToY(double latitude) const;
    double longitudeToX(double longitude) const;

private:
    QString m_name;
    QString m_system;
    double m_width{0.0};
    double m_height{0.0};
    double m_ha{1.0};
    double m_northAngle{0.0};
    double m_latitudeMin{0.0};
    double m_longitudeMin{0.0};
    double m_latitudeMax{0.0};
    double m_longitudeMax{0.0};
    double m_latitudeDegreesPerMeter{0.0};
    double m_longitudeDegreesPerMeter{0.0};
};

class View
{
public:
    View();

    // Dimensions
    double width() const { return m_width; }
    double height() const { return m_height; }
    double dpi() const { return m_dpi; }

    void setDimensions(int w, int h, double dpi);
    void setSiteDimensions(int w, int h);

    double margin() const { return m_margin; }
    void setMargin(double value);

    // Zoom
    double zoom() const { return m_zoom; }
    double zoomMin() const { return m_zoomMin; }
    void setZoom(double value, double pinchScale, double rx, double ry);

    // Translation
    double translatationX() const { return m_translationX; }
    double translatationY() const { return m_translationY; }

    void translate(double dx, double dy);
    void translateToSee(double x, double y, double r, double s, double widthRatio = 0.5);
    void resetTranslation();
    void updateTranslationMinMax();

    // Location
    void setLocationAvailable(bool enabled);
    bool isLocationAvailable() const { return m_locationAvailable; }

    void setLocation(double latitude, double longitude);
    double latitude() const { return m_latitude; }
    double longitude() const { return m_longitude; }

    void setLocationXY(double x, double y);
    double locationX() const { return m_locationX; }
    double locationY() const { return m_locationY; }

    void setAzimuthAvailable(bool enabled) { m_azimuthAvailable = enabled; }
    bool isAzimuthAvailable() const { return m_azimuthAvailable; }

    void setAzimuth(double azimuth) { m_azimuth = azimuth; }
    double azimuth() const { return m_azimuth; }

    // Options
    void setTreeScale(double scale);
    double treeScale() const { return m_treeScale; }

    void setShowMapTreeSpeciesLegend(bool b);
    bool showMapTreeSpeciesLegend() const { return m_showMapTreeSpeciesLegend; }

    void setMoveMapTreeSpeciesLegend(bool b);
    bool moveMapTreeSpeciesLegend() const { return m_moveMapTreeSpeciesLegend; }

    // Hotspots
    void setHotspotsReason(TreeSelectionReasonValue reason);
    TreeSelectionReasonValue hotspotsReason() const { return m_hotspotsReason; }

    void setHotspotsReasonText(const QString &reason);
    const QString &hotspotsReasonText() const { return m_hotspotsReasonText; }

    // Travailoscope
    void setValuationReasonText(const QString &reason);
    const QString &valuationReasonText() const { return m_valuationReasonText; }

private:
    double m_width{0.0};
    double m_height{0.0};
    double m_dpi{100.0};
    double m_margin{0.0};
    double m_zoom{0.95};
    double m_zoomMin{0.95};
    double m_siteWidth{0.0};
    double m_siteHeight{0.0};
    double m_translationX{0.0};
    double m_translationY{0.0};
    double m_translationXMin{0.0};
    double m_translationYMin{0.0};
    double m_translationXMax{0.0};
    double m_translationYMax{0.0};
    bool m_locationAvailable{false};
    double m_latitude{0.0};
    double m_longitude{0.0};
    double m_locationX{0.0};
    double m_locationY{0.0};
    bool m_azimuthAvailable{false};
    double m_azimuth{0.0};
    double m_treeScale{1.0};
    bool m_showMapTreeSpeciesLegend{true};
    bool m_moveMapTreeSpeciesLegend{true};
    TreeSelectionReasonValue m_hotspotsReason{TreeSelectionReasonValue::None};
    QString m_hotspotsReasonText{"None"};
    QString m_valuationReasonText{"None"};
};

class Table
{
public:
    Table() = default;

    void insert(const QString &columnHeader,
                const std::vector<QString> &columnValues)
    {
        m_data[columnHeader] = columnValues;
    }

    size_t rowCount() const
    {
        return m_data.size() > 0 ? m_data.cbegin()->second.size() : 0;
    }

    size_t colCount() const
    {
        return m_data.size();
    }

    void clear()
    {
        m_data.clear();
    }

    bool hasColumn(const QString &header) const
    {
        return m_data.count(header) > 0;
    }

    const std::vector<QString> &col(const QString &header) const
    {
        if (hasColumn(header))
        {
            return m_data.at(header);
        }

        return m_emptyColumn;
    }

    const QString &text(const QString &header, size_t row) const
    {
        const auto &c = col(header);
        if (row < c.size())
        {
            return c[row];
        }

        return m_emptyCell;
    }

    int toInt(const QString &header, size_t row) const
    {
        QString str = text(header, row);
        bool ok{false};
        return str.toInt(&ok);
    }

    uint64_t toULongLong(const QString &header, size_t row) const
    {
        QString str = text(header, row);
        bool ok{false};
        return str.toULongLong(&ok);
    }

    double toDouble(const QString &header,
                    size_t row,
                    bool hasDefaultValue = false,
                    double defaultValue = 1.0) const
    {
        QString str = text(header, row);
        bool ok{false};
        double value = str.replace(",", ".").toDouble(&ok);
        if (!ok && hasDefaultValue)
        {
            value = defaultValue;
        }
        return value;
    }

private:
    std::map<QString, std::vector<QString>> m_data;
    std::vector<QString> m_emptyColumn;
    QString m_emptyCell;
};

#endif // C_TYPE_H
