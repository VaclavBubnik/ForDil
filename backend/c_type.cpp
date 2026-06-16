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

#include "c_type.h"
#include <QCoreApplication>
#include <QVariantMap>
#include <QImage>
#include <QDebug>
#include <algorithm>
#include <random>

// #define FORDIL_DEBUG_TYPE_TRAVAILLOSCOPE_QUIZ 1
// #define FORDIL_DEBUG_TYPE_TYPOLOGY 1

void Core::clear()
{
    m_translations.clear();
}

void Core::setLanguageByCode(const QString &code)
{
    m_languageCode = code;
}

void Core::insertTranslation(const QString &source, const QString &translation, const QString &languageCode)
{
    m_translations[source].language[languageCode] = translation;
}

const QString &Core::translate(const QString &text) const
{
    auto itSource = m_translations.find(text);
    if (itSource != m_translations.end())
    {
        const Translation &translation = itSource->second;
        auto itLanguage = translation.language.find(m_languageCode);
        if (itLanguage != translation.language.end())
        {
            return itLanguage->second;
        }
    }

    return text;
}

Core &core()
{
    static Core instance;
    return instance;
}

Species::Species()
{
    // empty
}

Species::Species(uint64_t speciesId,
                 const QString &fullName,
                 const QColor &color,
                 const std::map<QString, double> &price)
  : m_speciesId(speciesId),
    m_fullName(fullName),
    m_color(color),
    m_price(price)
{
    // empty
}

double Species::price(const QString &qual) const
{
    auto it = m_price.find(qual);
    if (it != m_price.end())
    {
        return it->second;
    }

    return 0.0;
}

double Species::cv(const QString &qual, double volume) const
{
    return price(qual) * volume;
}

const std::vector<TreeSelectionReason> treeSelectionReasonMap = {
    {TreeSelectionReasonValue::None, "None", "#A6A6A6"},
    {TreeSelectionReasonValue::Competition, "Competition", "#008000"},
    {TreeSelectionReasonValue::Maturity, "Maturity", "#CC6600"},
    {TreeSelectionReasonValue::PromoteRegeneration, "Promote regeneration", "#99FF33"},
    {TreeSelectionReasonValue::PromoteRareSpecies, "Promote rare species", "#00B0F0"},
    {TreeSelectionReasonValue::Sanitary, "Sanitary", "#FF0000"},
    {TreeSelectionReasonValue::Technical, "Technical", "#9900FF"},
    {TreeSelectionReasonValue::BadQuality, "Bad quality", "#76b7b2"},
    {TreeSelectionReasonValue::Siblings, "Siblings", "#f28e2b"},
    {TreeSelectionReasonValue::Biodiversity, "Biodiversity", "#76b7b2"},
    {TreeSelectionReasonValue::HabitatTree, "Habitat tree", "#ff9da7"},
    {TreeSelectionReasonValue::DeadTree, "Dead tree", "#bab0ab"}
};

TreeSelectionReasonValue treeSelectionReasonFromString(const QString &str)
{
    for (const auto &it : treeSelectionReasonMap)
    {
        if (it.label == str)
        {
            return it.value;
        }
    }

    throw std::invalid_argument("Invalid reason string: " + str.toStdString());
}

QString treeSelectionReasonToString(TreeSelectionReasonValue reason)
{
    for (const auto &it : treeSelectionReasonMap)
    {
        if (it.value == reason)
        {
            return it.label;
        }
    }

    return "Invalid";
}

QString treeSelectionReasonToStringTr(TreeSelectionReasonValue reason)
{
    if (reason == TreeSelectionReasonValue::None)
    {
        return QCoreApplication::translate("Backend", "None");
    }

    if (reason == TreeSelectionReasonValue::Competition)
    {
        return QCoreApplication::translate("Backend", "Competition");
    }

    if (reason == TreeSelectionReasonValue::Maturity)
    {
        return QCoreApplication::translate("Backend", "Maturity");
    }

    if (reason == TreeSelectionReasonValue::PromoteRegeneration)
    {
        return QCoreApplication::translate("Backend", "Promote regeneration");
    }

    if (reason == TreeSelectionReasonValue::PromoteRareSpecies)
    {
        return QCoreApplication::translate("Backend", "Promote rare species");
    }

    if (reason == TreeSelectionReasonValue::Sanitary)
    {
        return QCoreApplication::translate("Backend", "Sanitary");
    }

    if (reason == TreeSelectionReasonValue::Technical)
    {
        return QCoreApplication::translate("Backend", "Technical");
    }

    if (reason == TreeSelectionReasonValue::BadQuality)
    {
        return QCoreApplication::translate("Backend", "Bad quality");
    }

    if (reason == TreeSelectionReasonValue::Siblings)
    {
        return QCoreApplication::translate("Backend", "Siblings");
    }

    if (reason == TreeSelectionReasonValue::Biodiversity)
    {
        return QCoreApplication::translate("Backend", "Biodiversity");
    }

    if (reason == TreeSelectionReasonValue::HabitatTree)
    {
        return QCoreApplication::translate("Backend", "Habitat tree");
    }

    if (reason == TreeSelectionReasonValue::DeadTree)
    {
        return QCoreApplication::translate("Backend", "Dead tree");
    }

    return QCoreApplication::translate("Backend", "Invalid");
}

QColor treeSelectionReasonColor(TreeSelectionReasonValue reason)
{
    for (const auto &it : treeSelectionReasonMap)
    {
        if (it.value == reason)
        {
            return it.color;
        }
    }

    return QColor("#000000");
}

TreeSelectionChoices::TreeSelectionChoices()
{
    // empty
}

TreeSelectionChoices::TreeSelectionChoices(bool competitionChecked,
                                           bool maturityChecked,
                                           bool promoteRegenerationChecked,
                                           bool promoteRareSpeciesChecked,
                                           bool sanitaryChecked,
                                           bool technicalChecked,
                                           bool badQualityChecked)
  : m_competitionChecked(competitionChecked),
    m_maturityChecked(maturityChecked),
    m_promoteRegenerationChecked(promoteRegenerationChecked),
    m_promoteRareSpeciesChecked(promoteRareSpeciesChecked),
    m_sanitaryChecked(sanitaryChecked),
    m_technicalChecked(technicalChecked),
    m_badQualityChecked(badQualityChecked)
{
    // empty
}

bool TreeSelectionChoices::checked(TreeSelectionReasonValue cuttingChoice) const
{
    switch (cuttingChoice)
    {
        case TreeSelectionReasonValue::Competition: return competitionChecked();
        case TreeSelectionReasonValue::Maturity: return maturityChecked();
        case TreeSelectionReasonValue::PromoteRegeneration: return promoteRegenerationChecked();
        case TreeSelectionReasonValue::PromoteRareSpecies: return promoteRareSpeciesChecked();
        case TreeSelectionReasonValue::Sanitary: return sanitaryChecked();
        case TreeSelectionReasonValue::Technical: return technicalChecked();
        case TreeSelectionReasonValue::BadQuality: return badQualityChecked();
        default: return false;
    }
}

bool TreeSelectionChoices::isSelected() const
{
    return m_competitionChecked ||
           m_maturityChecked ||
           m_promoteRegenerationChecked ||
           m_promoteRareSpeciesChecked ||
           m_sanitaryChecked ||
           m_technicalChecked ||
           m_badQualityChecked;
}

void TreeSelectionChoices::reset()
{
    m_competitionChecked = false;
    m_maturityChecked = false;
    m_promoteRegenerationChecked = false;
    m_promoteRareSpeciesChecked = false;
    m_sanitaryChecked = false;
    m_technicalChecked = false;
    m_badQualityChecked = false;
}

TreeLog::TreeLog()
{
    // empty
}

TreeLog::TreeLog(uint64_t treeId,
                 const QString &qual,
                 double vol,
                 double cv,
                 double yGain)
  : m_treeId(treeId),
    m_qual(qual),
    m_vol(vol),
    m_cv(cv),
    m_yGain(yGain)
{
    // empty
}

void TreeLog::add(const TreeLog &other)
{
    m_vol += other.vol();
    m_cv += other.cv();
    m_yGain += other.yGain();
}

EcologicalValue::EcologicalValue()
{
    // empty
}

EcologicalValue::EcologicalValue(uint64_t ecologicalValueId,
                                 const QString &theme,
                                 const QString &description,
                                 double nat,
                                 double chiro,
                                 double avri,
                                 double ento,
                                 double global)
  : m_ecologicalValueId(ecologicalValueId),
    m_theme(theme),
    m_description(description),
    m_nat(nat),
    m_chiro(chiro),
    m_avri(avri),
    m_ento(ento),
    m_global(global)
{
    // empty
}

Tree::Tree()
{
    // empty
}

Tree::Tree(uint64_t treeId,
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
           const std::vector<int> &dmh)
  : m_treeId(treeId),
    m_speciesId(speciesId),
    m_latitude(latitude),
    m_longitude(longitude),
    m_gridId(gridId),
    m_dbh(dbh),
    m_dbh10(dbh10),
    m_height(height),
    m_volumeQual(volumeQual),
    m_cv(cv),
    m_yGain(yGain),
    m_option(option),
    m_ecovalueGlob(ecovalueGlob),
    m_q4Prosilvalue(q4Prosilvalue),
    m_dead(dead),
    m_bio(bio),
    m_dmh(dmh)
{
    // empty
}

void Tree::setPosition(double x, double y)
{
    m_x = x;
    m_y = y;
}

double Tree::gha() const
{
    return M_PI * std::pow(dbh() * 0.5, 2);
}

void Tree::setQ4Prosilvalue(double q4Prosilvalue)
{
    m_q4Prosilvalue = q4Prosilvalue;
}

bool Tree::insertTreeLog(const TreeLog &treeLog)
{
    auto it = m_treeLog.find(treeLog.qual());
    if (it == m_treeLog.end())
    {
        m_treeLog[treeLog.qual()] = treeLog;
        return true;
    }

    it->second.add(treeLog);

    return false;
}

double Tree::vol(const QString &qual) const
{
    double result = 0;

    if (m_treeLog.empty())
    {
        for (const auto &[key, volume] : m_volumeQual)
        {
            if (qual == "" || qual == key)
            {
                result += volume;
            }
        }
    }
    else
    {
        for (const auto &[key, treeLog] : m_treeLog)
        {
            if (qual == "" || qual == key)
            {
                result += treeLog.vol();
            }
        }
    }

    return result;
}

double Tree::cv() const
{
    if (m_treeLog.empty())
    {
        return m_cv;
    }

    double result = 0;

    for (const auto &[key, treeLog] : m_treeLog)
    {
        result += treeLog.cv();
    }

    return result;
}

double Tree::yGain() const
{
    if (m_treeLog.empty())
    {
        return m_yGain;
    }

    double result = 0;

    for (const auto &[key, treeLog] : m_treeLog)
    {
        result += treeLog.yGain();
    }

    return result;
}

void Tree::resetSelection()
{
    m_selectionStatus = SelectionStatus::None;
}

void Tree::cancelSelection()
{
    m_selectionStatus = SelectionStatus::None;
    m_selected = false;

    m_treeSelectionChoices.reset();
}

void Tree::confirmSelection(const TreeSelectionChoices &treeSelectionChoices)
{
    m_selectionStatus = SelectionStatus::None;

    m_treeSelectionChoices = treeSelectionChoices;
    m_selected = m_treeSelectionChoices.isSelected();
}

HotspotTree::HotspotTree()
{
    // empty
}

HotspotTree::HotspotTree(uint64_t hotspotId,
                         uint64_t treeId,
                         const std::vector<QString> &cutOptions,
                         TreeSelectionReasonValue reason,
                         double latitude,
                         double longitude,
                         double radius,
                         const QString &drawings,
                         const QString &pedagogicalContents)
  : m_hotspotId(hotspotId),
    m_treeId(treeId),
    m_cutOptions(cutOptions),
    m_reason(reason),
    m_latitude(latitude),
    m_longitude(longitude),
    m_radius(radius),
    m_drawings(drawings),
    m_pedagogicalContents(pedagogicalContents)
{
    // empty
}

bool HotspotTree::hasCutOption(size_t idx) const
{
    if (idx < m_cutOptions.size())
    {
        return m_cutOptions[idx].size() > 0;
    }

    return false;
}

bool HotspotTree::cutOption(size_t idx) const
{
    if (idx < m_cutOptions.size())
    {
        bool ok{false};
        return (m_cutOptions[idx].toInt(&ok) == 0) ? false : true;
    }

    return false;
}

Hotspot::Hotspot()
{
    // empty
}

Hotspot::Hotspot(uint64_t hotspotId)
  : m_hotspotId(hotspotId)
{
    // empty
}

void Hotspot::push_back(const HotspotTree &hotspotTree)
{
    if (m_reason == TreeSelectionReasonValue::None)
    {
        m_reason = hotspotTree.reason();
    }

    if (hotspotTree.latitude() <= 90.0 && hotspotTree.longitude() <= 180.0)
    {
        m_latitude = hotspotTree.latitude();
        m_longitude = hotspotTree.longitude();
        m_radius = hotspotTree.radius();
    }

    if (m_drawings.isEmpty())
    {
        m_drawings = hotspotTree.drawings();
    }

    if (m_pedagogicalContents.isEmpty())
    {
        m_pedagogicalContents = hotspotTree.pedagogicalContents();
    }

    m_hotspotTreeList.push_back(hotspotTree);
}

std::map<uint64_t, bool> Hotspot::cutResults(const std::unordered_map<uint64_t, Tree> &trees) const
{
    std::array<std::map<uint64_t, bool>, 3> results;
    std::array<int, 3> correctResults = { 0, 0, 0 };

    for (const auto &hotspotTree : m_hotspotTreeList)
    {
        uint64_t treeId = hotspotTree.treeId();

        auto itTree = trees.find(treeId);
        if (itTree == trees.end())
        {
            qWarning() << "Cannot find hotspot treeId" << treeId;
            continue;
        }

        bool userCut = itTree->second.isSelected();

        for (size_t i = 0; i < correctResults.size(); i++)
        {
            if (!hotspotTree.hasCutOption(i))
            {
                results[i][treeId] = false;
                continue;
            }

            bool correctCut = true;
            bool hotspotCut = hotspotTree.cutOption(i);

            if (hotspotCut && !userCut)
            {
                correctCut = false;
            }

            if (userCut && !hotspotCut)
            {
                correctCut = false;
            }

            results[i][treeId] = correctCut;

            if (correctCut)
            {
                correctResults[i]++;
            }
        }
    }

    size_t max = 0;
    for (size_t i = 1; i < correctResults.size(); i++)
    {
        if (correctResults[i] > correctResults[max])
        {
            max = i;
        }
    }

    return results[max];
}

int TypologyGrid::struLoc(double lw,
                          double mw,
                          double bw,
                          double vbw,
                          double bwvbw)
{
    int result;

    if (lw > 50.0 && bwvbw <= 5.0)
    {
        result = 1;
    }
    else if (bwvbw <= 20.0)
    {
        if (mw <= 50.0)
        {
            if (lw > 50.0)
            {
                result = 2;
            }
            else
            {
                result = 3;
            }
        }
        else
        {
            result = 4;
        }
    }
    else if (bwvbw < 50.0)
    {
        if (mw <= 25.0)
        {
            result = 5;
        }
        else
        {
            if (lw >= 25.0)
            {
                result = 6;
            }
            else
            {
                result = 7;
            }
        }
    }
    else
    {
        if (bw > vbw)
        {
            result = 8;
        }
        else
        {
            result = 9;
        }
    }

    return result;
}

int TypologyGrid::ghaLoc(double gha)
{
    int result;

    if (gha < 2.0)
    {
        result = -1;
    }
    else if (gha < 5.0)
    {
        result = 0;
    }
    else if (gha < 10.0)
    {
        result = 1;
    }
    else if (gha < 15.0)
    {
        result = 2;
    }
    else if (gha < 20.0)
    {
        result = 3;
    }
    else if (gha < 25.0)
    {
        result = 4;
    }
    else if (gha < 30.0)
    {
        result = 5;
    }
    else
    {
        result = 6;
    }

    return result;
}

void TypologyLocal::update()
{
    DBH_lw = 0;
    DBH_mw = 0;
    DBH_bwvbw = 0;

    DBH_lw_10 = 0;
    DBH_bwvbw_10 = 0;

    Gha_loc = 0;

    if (grid.empty())
    {
        return;
    }

    for (const auto &it : grid)
    {
        DBH_lw += it.DBH_lw;
        DBH_mw += it.DBH_mw;
        DBH_bwvbw += it.DBH_bwvbw;

        DBH_lw_10 += it.DBH_lw_10;
        DBH_bwvbw_10 += it.DBH_bwvbw_10;

        Gha_loc += it.Gha_loc;
    }

    double n = static_cast<double>(grid.size());

    DBH_lw /= n;
    DBH_mw /= n;
    DBH_bwvbw /= n;

    DBH_lw_10 /= n;
    DBH_bwvbw_10 /= n;

    Gha_loc /= n;

#if defined(FORDIL_DEBUG_TYPE_TYPOLOGY)
    qDebug() << "local";
    qDebug() << "    lw      " << QString::number(DBH_lw, 'f', 1);
    qDebug() << "    mw      " << QString::number(DBH_mw, 'f', 1);
    qDebug() << "    bwvbw   " << QString::number(DBH_bwvbw, 'f', 1);
    qDebug() << "    lw_10   " << QString::number(DBH_lw_10, 'f', 1);
    qDebug() << "    bwvbw_10" << QString::number(DBH_bwvbw_10, 'f', 1);
#endif
}

void Typology::setGrid(double Sha_grid,
                       double Sha_tot,
                       size_t Nb_grid,
                       double Scale_y,
                       double North_angle)
{
    m_Sha_grid = Sha_grid;
    m_Sha_tot = Sha_tot;
    m_Nb_grid = Nb_grid;
    m_Scale_y = Scale_y;
    m_North_angle = North_angle;

    m_localInit.grid.resize(m_Nb_grid);
    m_localExpert.grid.resize(m_Nb_grid);
    m_localUser.grid.resize(m_Nb_grid);
}

void Typology::setWoodSize(double lw_from,
                           double mw_from,
                           double bw_from,
                           double vbw_from,
                           double bwvbw_from)
{
    m_lw_from = lw_from;
    m_mw_from = mw_from;
    m_bw_from = bw_from;
    m_vbw_from = vbw_from;
    m_bwvbw_from = bwvbw_from;
}

void Typology::setCutTreeIdsExpert(const std::set<uint64_t> &cutTreeIds)
{
    m_cutTreeIdsExpert = cutTreeIds;
}

void Typology::updateGrid(TypologyGrid &grid)
{
    if (grid.Typo_stru_loc == 1)
    {
        grid.color = QColor("#FAC421");
        grid.label = QCoreApplication::translate("Backend", "Dominant thin woods");
    }
    else if (grid.Typo_stru_loc == 2)
    {
        grid.color = QColor("#F5BD99");
        grid.label = QCoreApplication::translate("Backend", "Dominant thin and scattered big");
    }
    else if (grid.Typo_stru_loc == 3)
    {
        grid.color = QColor("#C7DFC7");
        grid.label = QCoreApplication::translate("Backend", "Thin and medium dominant woods");
    }
    else if (grid.Typo_stru_loc == 4)
    {
        grid.color = QColor("#629659");
        grid.label = QCoreApplication::translate("Backend", "Medium-sized dominant woods");
    }
    else if (grid.Typo_stru_loc == 5)
    {
        grid.color = QColor("#8157A2");
        grid.label = QCoreApplication::translate("Backend", "Thin and large dominant woods");
    }
    else if (grid.Typo_stru_loc == 6)
    {
        grid.color = QColor("#F6D3E5");
        grid.label = QCoreApplication::translate("Backend", "No dominant category");
    }
    else if (grid.Typo_stru_loc == 7)
    {
        grid.color = QColor("#BAD8EE");
        grid.label = QCoreApplication::translate("Backend", "Medium and large dominant woods");
    }
    else if (grid.Typo_stru_loc == 8)
    {
        grid.color = QColor("#0088CB");
        grid.label = QCoreApplication::translate("Backend", "Large dominant woods");
    }
    else if (grid.Typo_stru_loc == 9)
    {
        grid.color = QColor("#24408E");
        grid.label = QCoreApplication::translate("Backend", "Very large dominant woods");
    }
    else
    {
        grid.color = QColor("#cccccc");
        grid.label = QCoreApplication::translate("Backend", "Unknown");
    }
}

void Typology::calculateLocal(const std::unordered_map<uint64_t, Tree> &trees)
{
#if defined(FORDIL_DEBUG_TYPE_TYPOLOGY)
    qDebug() << "calculateLocal initial";
#endif

    calculateGrids(m_localInit, trees, {});

#if defined(FORDIL_DEBUG_TYPE_TYPOLOGY)
    qDebug() << "calculateLocal expert";
#endif

    calculateGrids(m_localExpert, trees, m_cutTreeIdsExpert);

#if defined(FORDIL_DEBUG_TYPE_TYPOLOGY)
    qDebug() << "calculateLocal user";
#endif

    std::set<uint64_t> cutTreeIdsUser;
    for (auto &[key, tree] : trees)
    {
        if (tree.isSelected())
        {
            cutTreeIdsUser.insert(tree.treeId());
        }
    }
    calculateGrids(m_localUser, trees, cutTreeIdsUser);
}

void Typology::calculateGrids(TypologyLocal &typology,
                              const std::unordered_map<uint64_t, Tree> &trees,
                              const std::set<uint64_t> &cutTreeIds)
{
#if defined(FORDIL_DEBUG_TYPE_TYPOLOGY)
    qDebug() << "calculateGrids grid count" << typology.grid.size();
#endif

    for (size_t i = 0; i < typology.grid.size(); i++)
    {
        TypologyGrid &grid = typology.grid[i];
        grid = TypologyGrid();
        grid.gridID = i + 1;
        calculateGrid(grid, trees, cutTreeIds);
        updateGrid(grid);
    }

    typology.update();
}

void Typology::calculateGrid(TypologyGrid &grid,
                             const std::unordered_map<uint64_t, Tree> &trees,
                             const std::set<uint64_t> &cutTreeIds)
{
    double gha = 0;

    double DBH_lw{0.0};
    double DBH_mw{0.0};
    double DBH_bw{0.0};
    double DBH_vbw{0.0};
    double DBH_bwvbw{0.0};

    double DBH_lw_10{0.0};
    double DBH_mw_10{0.0};
    double DBH_bw_10{0.0};
    double DBH_vbw_10{0.0};
    double DBH_bwvbw_10{0.0};

    for (auto &[key, tree] : trees)
    {
        if (tree.gridId() != grid.gridID)
        {
            continue;
        }

        if (tree.isDead())
        {
            continue;
        }

        if (cutTreeIds.count(tree.treeId()) > 0)
        {
#if defined(FORDIL_DEBUG_TYPE_TYPOLOGY)
            qDebug() << "skip cut tree" << tree.treeId();
#endif
            continue;
        }

        // stru 0
        double dbh = tree.dbh();

        if (dbh > m_vbw_from)
        {
            DBH_vbw += 1.0;
        }
        else if (dbh > m_bw_from)
        {
            DBH_bw += 1.0;
        }
        else if (dbh > m_mw_from)
        {
            DBH_mw += 1.0;
        }
        else if (dbh > m_lw_from)
        {
            DBH_lw += 1.0;
        }

        // stru 10
        double dbh10 = tree.dbh10();

        if (dbh10 > m_vbw_from)
        {
            DBH_vbw_10 += 1.0;
        }
        else if (dbh10 > m_bw_from)
        {
            DBH_bw_10 += 1.0;
        }
        else if (dbh10 > m_mw_from)
        {
            DBH_mw_10 += 1.0;
        }
        else if (dbh10 > m_lw_from)
        {
            DBH_lw_10 += 1.0;
        }

        // gha
        gha += tree.gha();
    }

    // stru 0
    double total = DBH_lw + DBH_mw + DBH_bw + DBH_vbw;
    if (total > 0.0)
    {
        grid.DBH_lw = (DBH_lw / total) * 100.0;
        grid.DBH_mw = (DBH_mw / total) * 100.0;
        grid.DBH_bw = (DBH_bw / total) * 100.0;
        grid.DBH_vbw = (DBH_vbw / total) * 100.0;
    }

    DBH_bwvbw = DBH_bw + DBH_vbw;
    grid.DBH_bwvbw = grid.DBH_bw + grid.DBH_vbw;

    grid.Typo_stru_loc = TypologyGrid::struLoc(grid.DBH_lw,
                                               grid.DBH_mw,
                                               grid.DBH_bw,
                                               grid.DBH_vbw,
                                               grid.DBH_bwvbw);

    // stru 10
    double total10 = DBH_lw_10 + DBH_mw_10 + DBH_bw_10 + DBH_vbw_10;
    if (total10 > 0.0)
    {
        grid.DBH_lw_10 = (DBH_lw_10 / total10) * 100.0;
        grid.DBH_mw_10 = (DBH_mw_10 / total10) * 100.0;
        grid.DBH_bw_10 = (DBH_bw_10 / total10) * 100.0;
        grid.DBH_vbw_10 = (DBH_vbw_10 / total10) * 100.0;
    }

    DBH_bwvbw_10 = DBH_bw_10 + DBH_vbw_10;
    grid.DBH_bwvbw_10 = grid.DBH_bw_10 + grid.DBH_vbw_10;

    grid.Typo_stru10_loc = TypologyGrid::struLoc(grid.DBH_lw_10,
                                                 grid.DBH_mw_10,
                                                 grid.DBH_bw_10,
                                                 grid.DBH_vbw_10,
                                                 grid.DBH_bwvbw_10);


    // gha
    gha /= m_Sha_grid;
    grid.Typo_gha_loc = TypologyGrid::ghaLoc(gha);

    grid.Gha_loc = gha;

#if defined(FORDIL_DEBUG_TYPE_TYPOLOGY)
    qDebug() << "grid" << grid.gridID;
    qDebug() << "  0" ;
    qDebug() << "    lw   " << QString::number(grid.DBH_lw, 'f', 2) << "count" << QString::number(DBH_lw, 'f', 0);
    qDebug() << "    mw   " << QString::number(grid.DBH_mw, 'f', 2) << "count" << QString::number(DBH_mw, 'f', 0);
    qDebug() << "    bw   " << QString::number(grid.DBH_bw, 'f', 2) << "count" << QString::number(DBH_bw, 'f', 0);
    qDebug() << "    vbw  " << QString::number(grid.DBH_vbw, 'f', 2) << "count" << QString::number(DBH_vbw, 'f', 0);
    qDebug() << "    bwvbw" << QString::number(grid.DBH_bwvbw, 'f', 2) << "count" << QString::number(DBH_bwvbw, 'f', 0);
    qDebug() << "    total" << QString::number(total, 'f', 0);
    qDebug() << "    Typo_stru_loc" << grid.Typo_stru_loc;
    qDebug() << "  10" ;
    qDebug() << "    lw   " << QString::number(grid.DBH_lw_10, 'f', 2) << "count" << QString::number(DBH_lw_10, 'f', 0);
    qDebug() << "    mw   " << QString::number(grid.DBH_mw_10, 'f', 2) << "count" << QString::number(DBH_mw_10, 'f', 0);
    qDebug() << "    bw   " << QString::number(grid.DBH_bw_10, 'f', 2) << "count" << QString::number(DBH_bw_10, 'f', 0);
    qDebug() << "    vbw  " << QString::number(grid.DBH_vbw_10, 'f', 2) << "count" << QString::number(DBH_vbw_10, 'f', 0);
    qDebug() << "    bwvbw" << QString::number(grid.DBH_bwvbw_10, 'f', 2) << "count" << QString::number(DBH_bwvbw_10, 'f', 0);
    qDebug() << "    total" << QString::number(total10, 'f', 0);
    qDebug() << "    Typo_stru_loc" << grid.Typo_stru10_loc;
    qDebug() << "  Typo_gha_loc" << grid.Typo_gha_loc << "gha" << gha;
#endif
}

void Typology::calculateGlobal()
{
    calculateGlobal(m_globalNature, m_localInit);
    calculateGlobal(m_globalExpert, m_localExpert);
    calculateGlobal(m_globalUser, m_localUser);
}

void Typology::calculateGlobal(TypologyGlobal &typologyGlobal,
                               const TypologyLocal &typologyLocal)
{
    typologyGlobal.Struc_n0_X = typologyLocal.DBH_lw;
    typologyGlobal.Struc_n0_Y = typologyLocal.DBH_bwvbw;

    typologyGlobal.Struc_n10_X = typologyLocal.DBH_lw_10;
    typologyGlobal.Struc_n10_Y = typologyLocal.DBH_bwvbw_10;

    typologyGlobal.Gha_n0 = typologyLocal.Gha_loc;
}

void Typology::calculateOverallAssessment()
{
}

Spot::Spot()
{
    // empty
}

Spot::Spot(uint64_t spotId,
           double latitude,
           double longitude,
           double radius)
  : m_spotId(spotId),
    m_x(0.0),
    m_y(0.0),
    m_latitude(latitude),
    m_longitude(longitude),
    m_radius(radius)
{
    // empty
}

void Spot::setPosition(double x, double y)
{
    m_x = x;
    m_y = y;
}

TravailloscopeAnswer::TravailloscopeAnswer()
{
    // empty
}

TravailloscopeAnswer::TravailloscopeAnswer(uint64_t questionId,
                                           uint64_t answerId,
                                           const QString &answerText,
                                           const QString &explanationText)
  : m_questionId(questionId),
    m_answerId(answerId),
    m_answerText(answerText),
    m_explanationText(explanationText)
{
    // empty
}

TravailloscopeSpotAnswer::TravailloscopeSpotAnswer()
{
    // empty
}

TravailloscopeSpotAnswer::TravailloscopeSpotAnswer(uint64_t spotId,
                                                   uint64_t questionId,
                                                   uint64_t answerId,
                                                   const QString &answerText,
                                                   const QString &explanationText,
                                                   bool correct)
  : m_spotId(spotId),
    m_questionId(questionId),
    m_answerId(answerId),
    m_answerText(answerText),
    m_explanationText(explanationText),
    m_correct(correct)
{
    // empty
}

TravailloscopeSpot::TravailloscopeSpot()
{
    // empty
}

TravailloscopeSpot::TravailloscopeSpot(uint64_t spotId)
  : m_spotId(spotId), m_score(0)
{
    // empty
}

void TravailloscopeSpot::clearAnswers()
{
    m_answers.clear();
    setFinished(false);
    setScore(0);
}

void TravailloscopeSpot::setAnswer(uint64_t questionId, const std::set<uint64_t> &answer)
{
    m_answers[questionId] = answer;
}

// TravailloscopeQuiz
void TravailloscopeQuiz::insert(uint64_t questionId, const QString &questionHeader)
{
    m_questions[questionId] = questionHeader;
}

void TravailloscopeQuiz::insert(const TravailloscopeAnswer &answer)
{
    m_answers[answer.questionId()].push_back(answer);
}

void TravailloscopeQuiz::insert(const TravailloscopeSpotAnswer &answer)
{
    m_spotAnswers[answer.spotId()].push_back(answer);
}

void TravailloscopeQuiz::insert(const TravailloscopeSpot &spot)
{
    m_spots[spot.spotId()] = spot;
}

void TravailloscopeQuiz::shuffle()
{
    for (auto &[spotId, answers] : m_spotAnswers)
    {
        shuffleById(answers, 1);
    }
}

void TravailloscopeQuiz::shuffleById(std::vector<TravailloscopeSpotAnswer> &list, uint64_t questionId)
{
    // Collect indices of items with given id
    std::vector<size_t> indices;
    for (size_t i = 0; i < list.size(); ++i)
    {
        if (list[i].questionId() == questionId)
        {
            indices.push_back(i);
        }
    }

    if (indices.size() <= 1)
    {
        return;
    }

    // Extract matching items
    std::vector<TravailloscopeSpotAnswer> temp;
    temp.reserve(indices.size());
    for (size_t idx : indices)
    {
        temp.push_back(list[idx]);
    }

    // Shuffle them
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(temp.begin(), temp.end(), gen);

    // Put them back and update sequence
    for (size_t i = 0; i < indices.size(); ++i)
    {
        temp[i].updateAnswerId(i + 1);
        list[indices[i]] = std::move(temp[i]);
    }
}

void TravailloscopeQuiz::clear()
{
    m_questions.clear();
    m_answers.clear();
    m_spotAnswers.clear();
    m_spots.clear();
    m_totalScore = 0;
}

void TravailloscopeQuiz::clearAnswers(uint64_t spotId)
{
    auto itSpot = m_spots.find(spotId);
    if (itSpot == m_spots.end())
    {
        return;
    }

    itSpot->second.clearAnswers();
}

void TravailloscopeQuiz::setAnswer(uint64_t spotId, uint64_t questionId, const std::set<uint64_t> &answer)
{
    // spot
    auto itSpot = m_spots.find(spotId);
    if (itSpot == m_spots.end())
    {
        return;
    }

    itSpot->second.setAnswer(questionId, answer);

    // spot answers : std::vector<TravailloscopeSpotAnswer>
    auto itSpotAnswers = m_spotAnswers.find(spotId);
    bool found = false;

    if (itSpotAnswers != m_spotAnswers.end())
    {
        for (const auto &a : itSpotAnswers->second)
        {
            if (a.questionId() > questionId)
            {
                found = true;
                break;
            }
        }
    }

    if (!found)
    {
        itSpot->second.setFinished(true);
    }
}

bool TravailloscopeQuiz::isFinished(uint64_t spotId) const
{
    auto itSpot = m_spots.find(spotId);
    if (itSpot == m_spots.end())
    {
        return false;
    }

    return itSpot->second.isFinished();
}

bool TravailloscopeQuiz::hasQuestion(uint64_t spotId, uint64_t questionId) const
{
    auto itSpotAnswers = m_spotAnswers.find(spotId);
    if (itSpotAnswers != m_spotAnswers.end())
    {
        for (const auto &a : itSpotAnswers->second)
        {
            if (a.questionId() == questionId)
            {
                return true;
            }
        }
    }

    return false;
}

int TravailloscopeQuiz::calculateScore(uint64_t spotId, uint64_t questionId) const
{
    int score = 0;

    // spot
    auto itSpot = m_spots.find(spotId);
    if (itSpot == m_spots.end())
    {
        return score;
    }

    // user : std::set<uint64_t> with answerIds for question
    auto itUserAnswers = itSpot->second.answers().find(questionId);
    if (itUserAnswers == itSpot->second.answers().end())
    {
        return score;
    }

    std::set<uint64_t> userAnswers = itUserAnswers->second; // answerId
    // search free text
    //     questionId 1
    //     userAnswers answerId [2]
    //
    // search predefined
    //     questionId 4
    //     userAnswers answerId [5, 7]

    // spot answers : std::vector<TravailloscopeSpotAnswer>
    auto itSpotAnswers = m_spotAnswers.find(spotId);
    if (itSpotAnswers == m_spotAnswers.end())
    {
        return score;
    }

    std::set<uint64_t> correctAnswers;

    // questionId answerId text correct
    //          1        1 foo      
    //          1        2 bar  true
    //          2        4      true
    //          3        2      true
    //          3        6      true

    for (const auto &a : itSpotAnswers->second)
    {
        if (a.questionId() == questionId && a.isCorrect())
        {
            correctAnswers.insert(a.answerId());
        }
    }

    if (userAnswers == correctAnswers)
    {
        // all answers are correct
        score = 2;
    }
    else
    {
        for (auto userAnswerId : userAnswers)
        {
            if (correctAnswers.count(userAnswerId) > 0)
            {
                // at least one answer is correct
                score = 1;
                break;
            }
        }
    }

    return score;
}

int TravailloscopeQuiz::calculateScore(uint64_t spotId) const
{
    int totalSpotScore = 0;
    int maxSpotScore = 0;

    for (uint64_t questionId = 1; questionId < 5; questionId++)
    {
        if (!hasQuestion(spotId, questionId))
        {
            break;
        }

        if (questionId > 2)
        {
            totalSpotScore += calculateScore(spotId, questionId);
            maxSpotScore += 2;
        }
    }

    if (totalSpotScore < 1)
    {
        return 0;
    }

    if (totalSpotScore < maxSpotScore)
    {
        return 1;
    }

    return 2;
}

void TravailloscopeQuiz::calculateScore()
{
    m_totalScore = 0;

    for (auto &it : m_spots)
    {
        int score = calculateScore(it.first);
        it.second.setScore(score);

#if defined(FORDIL_DEBUG_TYPE_TRAVAILLOSCOPE_QUIZ)
        qDebug() << "spot" << it.first << "score" << score;
#endif

        m_totalScore += score;
    }

#if defined(FORDIL_DEBUG_TYPE_TRAVAILLOSCOPE_QUIZ)
    qDebug() << "total score" << m_totalScore;
#endif
}

int TravailloscopeQuiz::score(uint64_t spotId) const
{
    auto itSpot = m_spots.find(spotId);
    if (itSpot == m_spots.end())
    {
        return 0;
    }

    return itSpot->second.score();
}

QString TravailloscopeQuiz::scoreColor(int score)
{
    if (score < 1)
    {
        return "#000000";
    }

    if (score < 2)
    {
        return "#FFC616";
    }

    return "#9AD35D";
}

QString TravailloscopeQuiz::scoreText(int score, size_t max)
{
    double pt = static_cast<double>(score) * 0.5;

    QString str = QString::number(pt, 'f', 1);
    if (str.endsWith(".0"))
    {
        str.chop(2);
    }

    return str + " / " + QString::number(max);
}

QString TravailloscopeQuiz::totalScoreText() const
{
#if defined(FORDIL_DEBUG_TYPE_TRAVAILLOSCOPE_QUIZ)
    qDebug() << "get total score" << m_totalScore;
#endif
    return TravailloscopeQuiz::scoreText(m_totalScore, m_spots.size());
}

QString TravailloscopeQuiz::totalScoreColor() const
{
    return "#ffffff";
}

QVariantList TravailloscopeQuiz::scoreModelData() const
{
    QVariantList list;

    for (const auto &it : m_spots)
    {
        list << QVariantMap{{"label", spotNo(it.first)},
                            {"score", scoreText(it.second.score(), 1)},
                            {"color", scoreColor(it.second.score())}};
    }

    return list;
}

QString TravailloscopeQuiz::scoreTable() const
{
    QString html;

    html += "<table>";

    html += "<tr>";
    html += "<td>" + QCoreApplication::translate("Backend", "Score") + "</td>";
    html += "<td>" + totalScoreText() + "</td>";
    html += "</tr>";

    for (const auto &it : m_spots)
    {
        QString label = spotNo(it.first);
        QString score = scoreText(it.second.score(), 1);
        QString color = scoreColor(it.second.score());

        QString row;

        row += "<tr>";
        row += "<td>" + label + "</td>";
        row += "<td><span style='color:" + color + ";'>" + score + "</span></td>";
        row += "</tr>";

        html += row;
    }

    html += "</table>";

    return html;
}

std::vector<uint64_t> TravailloscopeQuiz::spotIdList() const
{
    std::vector<uint64_t> list;

    for (const auto &it : m_spots)
    {
        list.push_back(it.first);
    }

    return list;
}

QString TravailloscopeQuiz::spotNo(uint64_t spotId) const
{
    return QCoreApplication::translate("Backend", "Spot n°") +
           QString::number(spotId);
}

QString TravailloscopeQuiz::questionHeader(uint64_t questionId) const
{
    auto it = m_questions.find(questionId);
    if (it == m_questions.end())
    {
        return "Question " + QString::number(questionId);
    }

    return core().translate(it->second);
}

std::map<uint64_t, QString> TravailloscopeQuiz::answerText(uint64_t spotId, uint64_t questionId) const
{
    std::map<uint64_t, QString> map;

    // spot answers : std::vector<TravailloscopeSpotAnswer>
    auto itSpotAnswers = m_spotAnswers.find(spotId);
    if (itSpotAnswers == m_spotAnswers.end())
    {
#if defined(FORDIL_DEBUG_TYPE_TRAVAILLOSCOPE_QUIZ)
        qDebug() << "no spotId" << spotId;
#endif
        return map;
    }

    // free text answers
    bool freeTextFound = false;

    for (const auto &a : itSpotAnswers->second)
    {
        if (a.questionId() != questionId)
        {
            continue;
        }

        if (!a.answerText().isEmpty())
        {
            // free text answer
            freeTextFound = true;
            map[a.answerId()] = core().translate(a.answerText());
        }
    }

    if (freeTextFound)
    {
        return map;
    }

    // standard answers
    auto itAnswers = m_answers.find(questionId);
    if (itAnswers == m_answers.end())
    {
#if defined(FORDIL_DEBUG_TYPE_TRAVAILLOSCOPE_QUIZ)
        qDebug() << "no questionId" << questionId;
#endif
        return map;
    }

    for (const auto &a : itAnswers->second)
    {
        map[a.answerId()] = core().translate(a.answerText());
    }

    return map;
}

QVariantList TravailloscopeQuiz::checkBoxItems(uint64_t spotId, uint64_t questionId) const
{
    QVariantList list;

    auto map = answerText(spotId, questionId);

    for (const auto &it : map)
    {
        list << QVariantMap{{"id", QString::number(it.first)},
                            {"text", it.second}};
    }

#if defined(FORDIL_DEBUG_TYPE_TRAVAILLOSCOPE_QUIZ)
    qDebug() << "checkBoxItems" << list;
#endif

    return list;
}

QString TravailloscopeQuiz::answerText(uint64_t spotId, uint64_t questionId, uint64_t answerId, bool &correct) const
{
    // correct
    correct = false;
    auto itSpotAnswers = m_spotAnswers.find(spotId);
    if (itSpotAnswers != m_spotAnswers.end())
    {
        for (const auto &a : itSpotAnswers->second)
        {
            if (a.questionId() == questionId && a.answerId() == answerId && a.isCorrect())
            {
                correct = true;
                break;
            }
        }
    }

    // Text
    std::map<uint64_t, QString> map = answerText(spotId, questionId); // answerId:tr_text
    auto itAnswers = map.find(answerId);
    if (itAnswers != map.end())
    {
        return itAnswers->second;
    }

    return QString();
}

QString TravailloscopeQuiz::pedagogicalContentYourAnswers(uint64_t spotId, uint64_t questionId) const
{
    QString out;

    out += "<b>" + QCoreApplication::translate("Backend", "Your answer(s)") + " :</b>";

    auto itSpot = m_spots.find(spotId);
    if (itSpot == m_spots.end())
    {
        // no answers for this spot
        out += "<br><br>";
        return out;
    }

    auto itUserAnswers = itSpot->second.answers().find(questionId);
    if (itUserAnswers == itSpot->second.answers().end())
    {
        // no answers for this question
        out += "<br><br>";
        return out;
    }

    std::set<uint64_t> userAnswers = itUserAnswers->second; // answerId list

    out += "<ul>";
    for (auto answerId : userAnswers)
    {
        QString str;
        bool correct = false;

        str = answerText(spotId, questionId, answerId, correct);
        if (str.isEmpty())
        {
            continue;
        }

        if (correct)
        {
            out += "<li><span style='color:#6cda91;'>" + str + "</span></li>";
        }
        else
        {
            out += "<li><span style='color:#f86b6f;'>" + str + "</span></li>";
        }
    }
    out += "</ul>";

    return out;
}

QString TravailloscopeQuiz::pedagogicalContent(uint64_t spotId, uint64_t questionId) const
{
    QString out;

    QString hdr = "<h3 style='color:#257879; margin-top: 20px;'>"
                + questionHeader(questionId)
                + "</h3>";
    out = hdr;

    // your answers
    if (questionId > 2)
    {
        out += pedagogicalContentYourAnswers(spotId, questionId);
    }

    // spot answers
    auto itSpotAnswers = m_spotAnswers.find(spotId);
    if (itSpotAnswers == m_spotAnswers.end())
    {
        return out;
    }

    // correct answers
    out += "<b>" + QCoreApplication::translate("Backend", "The correct answer(s)") + " :</b>";
    out += "<ul>";

    for (const auto &a : itSpotAnswers->second)
    {
        if (a.questionId() != questionId || !a.isCorrect())
        {
            continue;
        }

        // a is the next correct answer in questionId
        QString str = a.answerText();
        if (str.isEmpty())
        {
            // find in definitions
            auto itAnswers = m_answers.find(questionId);
            if (itAnswers == m_answers.end())
            {
                continue;
            }

            for (const auto &b : itAnswers->second)
            {
                if (a.answerId() == b.answerId())
                {
                    str = b.answerText();
                    break;
                }
            }
        }

        if (str.isEmpty())
        {
            continue;
        }

        str = core().translate(str);

        out += "<li><span style='color:#6cda91;'>" + str + "</span></li>";
    }

    out += "</ul>";

    // pedagogical content
    for (const auto &a : itSpotAnswers->second)
    {
        if (a.questionId() != questionId || !a.isCorrect())
        {
            continue;
        }

        // a is the next correct answer in questionId
        QString str = a.answerText();
        QString strExplanation = a.explanationText();
        if (str.isEmpty())
        {
            // find in definitions
            auto itAnswers = m_answers.find(questionId);
            if (itAnswers == m_answers.end())
            {
                continue;
            }

            for (const auto &b : itAnswers->second)
            {
                if (a.answerId() == b.answerId())
                {
                    str = b.answerText();
                    strExplanation = b.explanationText();
                    break;
                }
            }
        }

        if (str.isEmpty())
        {
            continue;
        }

        str = core().translate(str);
        strExplanation = core().translate(strExplanation);

        out += "<b>" + str + "</b><br>";
        out += strExplanation;
        out += "<br><br>";
    }

    return out;
}

QString TravailloscopeQuiz::pedagogicalContents(uint64_t spotId, uint64_t questionId) const
{
    // When question is 0, then show all

    QString out;

    if (questionId == 0)
    {
        for (uint64_t i = 1; i < 5; i++)
        {
            QString content = pedagogicalContent(spotId, i);
            out += content;
        }
    }
    else
    {
        out += pedagogicalContent(spotId, questionId);
    }

    return out;
}


// DiscoveryTour
void DiscoveryTour::clear()
{
    m_states.clear();
}

void DiscoveryTour::insert(uint64_t stateId,
                           uint64_t stopId,
                           uint64_t treeId,
                           const QString &treeString,
                           const QString &header,
                           const QString &text)
{
    DiscoveryState &q = m_states[stateId];

    q.m_stateId = stateId;
    q.m_stopId = stopId;
    q.m_treeId = treeId;
    q.m_treeString = treeString;
    q.m_header = header;
    q.m_text = text;
}

int DiscoveryTour::stop(uint64_t stateId) const
{
    const auto it = m_states.find(stateId);
    if (it == m_states.end())
    {
        return 0;
    }

    return it->second.m_stopId;
}

uint64_t DiscoveryTour::treeId(uint64_t stateId) const
{
    const auto it = m_states.find(stateId);
    if (it == m_states.end())
    {
        return 0;
    }

    return it->second.m_treeId;
}

QString DiscoveryTour::message(uint64_t stateId) const
{
    const auto it = m_states.find(stateId);
    if (it == m_states.end())
    {
        return QString();
    }

    QString hdr1 = QCoreApplication::translate("Backend", "STOP No.");
    QString hdr2 = QCoreApplication::translate("Backend", "go in front of tree No.");

    return hdr1 +
           QString::number(it->second.m_stopId) + ": " +
           hdr2 + " " +
           it->second.m_treeString;
           // QString::number(it->second.m_treeId);
}

QString DiscoveryTour::header(uint64_t stateId) const
{
    const auto it = m_states.find(stateId);
    if (it == m_states.end())
    {
        return QString();
    }

    if (it->second.m_stopId > 0)
    {
        QString str = QCoreApplication::translate("Backend", "STOP No.");
        return str + QString::number(it->second.m_stopId);
    }

    return QCoreApplication::translate("Backend", "Information");
}

QString DiscoveryTour::text(uint64_t stateId) const
{
    const auto it = m_states.find(stateId);
    if (it == m_states.end())
    {
        return QString();
    }

    QString text = it->second.m_text;
    text = core().translate(text);
    text.replace("<c>", "<span style='color:#54b191;'>");
    text.replace("</c>", "</span>");

    return text;
}

void DiscoveryQuiz::clear()
{
    m_questions.clear();
}

void DiscoveryQuiz::insert(uint64_t questionId,
                           uint64_t optionId,
                           const QString &type,
                           const QString &imageName,
                           const QString &text)
{
    DiscoveryQuestion &q = m_questions[questionId];

    if (type == "Question")
    {
        q.m_question = text;
    }
    else if (type == "Text")
    {
        q.m_questionText = text;
    }
    else if (type == "Answer")
    {
        q.m_answer.m_optionId = optionId;
        q.m_answer.m_imageName = imageName;
        q.m_answer.m_text = text;
    }
    else
    {
        DiscoveryOption option;
        option.m_optionId = optionId;
        option.m_imageName = imageName;
        option.m_text = text;
        q.m_options.push_back(std::move(option));
    }
}

QString DiscoveryQuiz::questionHeader(uint64_t questionId) const
{
    const auto it = m_questions.find(questionId);
    if (it == m_questions.end())
    {
        return QString();
    }

    QString header = QCoreApplication::translate("Backend", "Quiz #");

    return header +
           QString::number(questionId) + ": " +
           core().translate(it->second.m_question);
}

QString DiscoveryQuiz::questionText(uint64_t questionId) const
{
    const auto it = m_questions.find(questionId);
    if (it == m_questions.end())
    {
        return QString();
    }

    return core().translate(it->second.m_questionText);
}

QString DiscoveryQuiz::answerHeader(uint64_t questionId) const
{
    const auto it = m_questions.find(questionId);
    if (it == m_questions.end())
    {
        return QString();
    }

    QString header = QCoreApplication::translate("Backend", "Answer of quiz n°");

    return header + QString::number(questionId);
}

QString DiscoveryQuiz::answerText(uint64_t questionId) const
{
    const auto it = m_questions.find(questionId);
    if (it == m_questions.end())
    {
        return QString();
    }

    const DiscoveryOption &answer = it->second.m_answer;
    QString str;

    if (!answer.m_imageName.isEmpty())
    {
        str += "<img src='qrc:/images/" + answer.m_imageName + "' width='1'/><br>";
    }
    
    str += core().translate(answer.m_text);

    return str;
}

void ResultExplanations::clear()
{
    m_explanations.clear();
}

void ResultExplanations::insert(const QString &result,
                                const QString &explanation)
{
    m_explanations[result] = explanation;
}

QString ResultExplanations::explanation(const QString &result) const
{
    const auto it = m_explanations.find(result);
    if (it == m_explanations.end())
    {
        return QString();
    }

    return core().translate(it->second);
}

SelectedTree::SelectedTree()
{
    // empty
}

void SelectedTree::pick(const Tree &tree, const Species &species)
{
    m_hit = true;
    m_selected = tree.isSelected();
    
    m_treeId = tree.treeId();
    m_x = tree.x();
    m_y = tree.y();
    m_dbh = tree.dbh();
    m_speciesName = species.fullName();
    m_treeSelectionChoices = tree.treeSelectionChoices();

    m_gps = "Lat: " + QString::number(tree.latitude(), 'f', 6) +
            " Lon: " + QString::number(tree.longitude(), 'f', 6);
    m_latitude = tree.latitude();
    m_longitude = tree.longitude();
}

void SelectedTree::reset()
{
    m_hit = false;
}

SelectedHotspot::SelectedHotspot()
{
    // empty
}

void SelectedHotspot::pick(const Hotspot &hotspot)
{
    m_hit = true;
    m_hotspotId = hotspot.hotspotId();

    // Get data
#if defined(FORDIL_DEBUG_TYPE)
    qDebug() << "pick hotspotId" << m_hotspotId;
    qDebug() << hotspot.size() << "hotspot trees";
#endif

    // Header
    QString reason = treeSelectionReasonToStringTr(hotspot.reason());

    m_pedagogicalHeader = QCoreApplication::translate("Backend", "Hotspot") + " n°" + 
                          QString::number(m_hotspotId) + ": " + reason;

    // Body
    m_pedagogicalContents = core().translate(hotspot.pedagogicalContents());
}

void SelectedHotspot::reset()
{
    m_hit = false;
}

SelectedSpot::SelectedSpot()
{
    // empty
}

void SelectedSpot::pick(const Spot &spot)
{
    m_hit = true;

    m_spotId = spot.spotId();
    m_x = spot.x();
    m_y = spot.y();
    m_radius = spot.radius();
}

void SelectedSpot::reset()
{
    m_hit = false;
}

void SelectedStop::pick(uint64_t stateId)
{
    m_hit = true;
    m_stateId = stateId;
}

void SelectedStop::reset()
{
    m_hit = false;
}

SelectedPoint::SelectedPoint()
{
    // empty
}

void SelectedPoint::pick(double latitude, double longitude)
{
    m_latitude = latitude;
    m_longitude = longitude;
  
}

void SelectedPoint::reset()
{
}

Site::Site()
{
    // empty
}

Site::Site(const QString &name,
           const QString &system,
           double width,
           double height,
           double ha,
           double northAngle)
  : m_name(name),
    m_system(system),
    m_width(width),
    m_height(height),
    m_ha(ha),
    m_northAngle(northAngle),
    m_latitudeMin(0.0),
    m_longitudeMin(0.0),
    m_latitudeMax(0.0),
    m_longitudeMax(0.0)
{
    // empty
}

void Site::setDimension(double width, double height)
{
    m_width = width;
    m_height = height;
}

void Site::setLocation(double latitudeMin,
                       double longitudeMin,
                       double latitudeMax,
                       double longitudeMax,
                       double latitudeDegreesPerMeter,
                       double longitudeDegreesPerMeter)
{
    m_latitudeMin = latitudeMin;
    m_longitudeMin = longitudeMin;

    m_latitudeMax = latitudeMax;
    m_longitudeMax = longitudeMax;

    m_latitudeDegreesPerMeter = latitudeDegreesPerMeter;
    m_longitudeDegreesPerMeter = longitudeDegreesPerMeter;
}

double Site::latitudeToY(double latitude) const
{
    return (m_latitudeMax - latitude) / m_latitudeDegreesPerMeter;
}

double Site::longitudeToX(double longitude) const
{
    return (longitude - m_longitudeMin) / m_longitudeDegreesPerMeter;
}

View::View()
{
    // empty
}

void View::setDimensions(int w, int h, double dpi)
{
    m_width = static_cast<double>(w);
    m_height = static_cast<double>(h);
    m_dpi = dpi;

#if defined(FORDIL_DEBUG_TYPE)
    qDebug() << "view w" << w << "h" << h << "DPI" << m_dpi;
#endif
}

void View::setSiteDimensions(int w, int h)
{
    m_siteWidth = w;
    m_siteHeight = h;
}

void View::setMargin(double value)
{
    m_margin = value;
}

void View::setZoom(double value, double pinchScale, double rx, double ry)
{
    double currentVisibleSiteWidth = m_siteWidth / m_zoom;
    double currentVisibleSiteHeight = m_siteWidth / m_zoom;

    m_zoom = value;

    if (m_zoom < zoomMin())
    {
        m_zoom = zoomMin();
    }

    double newVisibleSiteWidth = m_siteWidth / m_zoom;
    double newVisibleSiteHeight = m_siteWidth / m_zoom;

    double dx = (currentVisibleSiteWidth - newVisibleSiteWidth) * rx;
    double dy = (currentVisibleSiteHeight - newVisibleSiteHeight) * ry;

    m_translationX -= dx;
    m_translationY -= dy;

    updateTranslationMinMax();
}

void View::translate(double dx, double dy)
{
    if (m_width < 1e-9)
    {
        return;
    }

    double r = m_siteWidth / (m_width * m_zoom);

    m_translationX += dx * r;
    m_translationY += dy * r;

    updateTranslationMinMax();

#if defined(FORDIL_DEBUG_TYPE)
    qDebug() << "translate" << dx << "," << dy << "=" << m_translationX << "," << m_translationY;
#endif
}

void View::translateToSee(double x, double y, double r, double s, double widthRatio)
{
    if (s > 0.0 && r > 0.0)
    {
        m_zoom = (15.0 / s) / r;
    }

    double currentVisibleSiteWidth = m_siteWidth / m_zoom;
    double currentVisibleSiteHeight = m_siteHeight / m_zoom;

    m_translationX = (currentVisibleSiteWidth * widthRatio) - x;
    m_translationY = (currentVisibleSiteHeight * 0.5) - y;

#if defined(FORDIL_DEBUG_TYPE)
    qDebug() << "translateToSee x" << x << "y" << y << "r" << r;
    qDebug() << "visible site w" << currentVisibleSiteWidth << "h" << currentVisibleSiteHeight;
    qDebug() << "translation x" << m_translationX << "y" << m_translationY;
    qDebug() << "zoom" << m_zoom;
#endif

    updateTranslationMinMax();
}

void View::resetTranslation()
{
    m_translationX = 0.0;
    m_translationY = 0.0;
}

void View::updateTranslationMinMax()
{
    double m = 20.0 / m_zoom;

    double w = m_siteWidth / m_zoom;
    double h = m_siteHeight / m_zoom;

    m_translationXMin = m - m_siteWidth;
    m_translationXMax = w - m;
    m_translationYMin = m - m_siteHeight;
    m_translationYMax = h - m;

#if defined(FORDIL_DEBUG_TYPE)
    qDebug() << "translation x min" << m_translationXMin << "max" << m_translationXMax;
    qDebug() << "translation y min" << m_translationYMin << "max" << m_translationYMax;
#endif

    if (m_translationX < m_translationXMin)
    {
        m_translationX = m_translationXMin;
    }

    if (m_translationX > m_translationXMax)
    {
        m_translationX = m_translationXMax;
    }

    if (m_translationY < m_translationYMin)
    {
        m_translationY = m_translationYMin;
    }

    if (m_translationY > m_translationYMax)
    {
        m_translationY = m_translationYMax;
    }
}

void View::setLocationAvailable(bool enabled)
{
    m_locationAvailable = enabled;
}

void View::setLocation(double latitude, double longitude)
{
    m_latitude = latitude;
    m_longitude = longitude;
}

void View::setLocationXY(double x, double y)
{
    m_locationX = x;
    m_locationY = y;
}

void View::setTreeScale(double scale)
{
    m_treeScale = scale;
}

void View::setShowMapTreeSpeciesLegend(bool b)
{
    m_showMapTreeSpeciesLegend = b;
}

void View::setMoveMapTreeSpeciesLegend(bool b)
{
    m_moveMapTreeSpeciesLegend = b;
}

void View::setHotspotsReason(TreeSelectionReasonValue reason)
{
    m_hotspotsReason = reason;
}

void View::setHotspotsReasonText(const QString &reason)
{
    m_hotspotsReasonText = reason;
}

void View::setValuationReasonText(const QString &reason)
{
    m_valuationReasonText = reason;
}
