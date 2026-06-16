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

#include "c_result.h"
#include <algorithm>
#include <QCoreApplication>
#include <QDebug>

#define C_RESULT_DBH_MIN 0.025
// #define FORDIL_DEBUG_RESULT_GINI

Result::Result()
{
    // empty
}

// This function calculates the tree distribution chart by DBH (Diameter at Breast Height).
// It prepares data for a StackedBarChart with two datasets:
//    "Cut" (selected trees) and
//    "Before" (uncut trees).
void Result::treesDistribution(StackedBarChart &result,
                                Data &data,
                                const QString &distributionOption)
{
    // Initialize the result
    result.items.clear();

    // Axes
    result.xLabel = QCoreApplication::translate("Backend", "DBH classes [cm]");

    if (distributionOption == "NHA")
    {
        result.yLabel = QCoreApplication::translate("Backend", "N/ha");
    }
    else
    {
        result.yLabel = QCoreApplication::translate("Backend", "G/ha");
    }

    // Set labels for the stacked bar chart
    result.sets.resize(2);
    result.sets[0].label = QCoreApplication::translate("Backend", "Uncut"); // Label for trees before cutting (remaining trees)
    result.sets[0].color = QColor("#41B280");
    result.sets[1].label = QCoreApplication::translate("Backend", "Cut");    // Label for trees that are selected (to be cut)
    result.sets[1].color = QColor("#FF0000");

    // Variable to store the maximum DBH value found
    double max = 0;

    // Iterate through all trees to find the maximum DBH
    for (auto &[key, tree] : data.trees())
    {
        if (!tree.isDead() && tree.dbh() > max)
        {
            max = tree.dbh();
        }
    }

    if (max < C_RESULT_DBH_MIN)
    {
        qWarning() << "DBH max" << max << "< min" << C_RESULT_DBH_MIN;
        return;
    }

#if defined(FORDIL_DEBUG_RESULT)
    qDebug() << "DBH max" << max;
#endif

    // Define the step size for DBH groups (e.g., 0-5, 5-10, 10-15, etc.)
    double step = 0.05;

    // Calculate the number of DBH categories needed
    size_t n = static_cast<size_t>(std::ceil((max - C_RESULT_DBH_MIN) / step));
#if defined(FORDIL_DEBUG_RESULT)
    qDebug() << "n" << n;
#endif

    // Resize the items array to hold all categories
    result.items.resize(n);

    // Prepare and initialize the result items
    for (size_t i = 0; i < n; i++)
    {
        result.items[i].category = QString::number(static_cast<double>(i + 1) * step * 100.0);
        result.items[i].values.resize(2);
        result.items[i].values[0] = 0; // Initialize uncut count to 0
        result.items[i].values[1] = 0; // Initialize selected count to 0
    }

    // Distribute trees into categories based on their DBH
#if defined(FORDIL_DEBUG_RESULT)
    qDebug() << "distributionOption" << distributionOption;
#endif
    bool useNha = distributionOption == "NHA";
    for (auto &[key, tree] : data.trees())
    {
        // Skip processing this tree if its diameter at breast height (DBH)
        // is less than the minimum threshold defined by C_RESULT_DBH_MIN.
        double x = tree.dbh() - C_RESULT_DBH_MIN;
        if (tree.isDead() || x < 0.0)
        {
            continue; // Move to the next tree in the loop
        }

        // Determine which category (bin) the tree falls into
        int i = static_cast<int>(std::floor(x / step));

        // Calculate the value
        double value = 0.0;
        if (useNha)
        {
            value = 1.0; // One tree
        }
        else
        {
            value = tree.gha();
        }

        if (tree.isSelected())
        {
            // If tree is selected (to be cut), increment "Cut" value
            result.items[i].values[1] += value;
        }
        else
        {
            // If tree is not selected, increment "Before" value
            result.items[i].values[0] += value;
        }
    }

    // Calculate N/ha
    for (size_t i = 0; i < n; i++)
    {
        result.items[i].values[0] /= data.site().ha();
        result.items[i].values[1] /= data.site().ha();
    }
}

double Result::thinningFactor(Data &data)
{
    // DBH before
    double sumDbhBefore = 0;
    double nDbhBefore = 0;

    // DBH cut
    double sumDbhCut = 0;
    double nDbhCut = 0;

    // Iterate through all trees
    for (auto &[key, tree] : data.trees())
    {
        sumDbhBefore += tree.dbh();
        nDbhBefore += 1.0;

        if (tree.isSelected())
        {
            sumDbhCut += tree.dbh();
            nDbhCut += 1.0;
        }
    }

    // Default result is N/A as Nan (for example, when nonthing is cut)
    double result = std::numeric_limits<double>::quiet_NaN();

    if (nDbhBefore > 0.0 && nDbhCut > 0.0 && sumDbhBefore > 0.0)
    {
        result = (sumDbhCut / nDbhCut) / (sumDbhBefore / nDbhBefore);
    }

    return result;
}

static double resultTreeDistributionIndexGini(const std::vector<double> &area)
{
    size_t n = area.size();
#if defined(FORDIL_DEBUG_RESULT) || defined(FORDIL_DEBUG_RESULT_GINI)
    qDebug() << "n" << n;
#endif
    if (n < 1)
    {
        return 0.0;
    }

    std::vector<double> areaSorted(area);
    // areaSorted.push_back(0.0);
    std::sort(areaSorted.begin(), areaSorted.end());
#if defined(FORDIL_DEBUG_RESULT_GINI)
    qDebug() << "areaSorted";
    for (size_t i = 0; i < n; i++)
    {
        qDebug() << "  " << i << ":" << QString::number(areaSorted[i], 'f', 8);
    }
#endif

    std::vector<double> areaAcc(n);
    areaAcc[0] = areaSorted[0];
    for (size_t i = 1; i < n; i++)
    {
        areaAcc[i] = areaAcc[i - 1] + areaSorted[i];
    }
#if defined(FORDIL_DEBUG_RESULT_GINI)
    qDebug() << "areaAcc";
    for (size_t i = 0; i < n; i++)
    {
        qDebug() << "  " << i << ":" << QString::number(areaAcc[i], 'f', 8);
    }
#endif

    double areaTotal = areaAcc[n - 1];
#if defined(FORDIL_DEBUG_RESULT) || defined(FORDIL_DEBUG_RESULT_GINI)
    qDebug() << "areaTotal" << areaTotal;
#endif
    if (areaTotal < 1e-9)
    {
        return 0.0;
    }

    std::vector<double> areaRel(n);
    for (size_t i = 0; i < n; i++)
    {
        areaRel[i] = areaAcc[i] / areaTotal;
    }
#if defined(FORDIL_DEBUG_RESULT_GINI)
    qDebug() << "areaRel";
    for (size_t i = 0; i < n; i++)
    {
        qDebug() << "  " << i << ":" << QString::number(areaRel[i], 'f', 8);
    }
#endif

    // areaRel
    // 0
    // 0.25
    // 0.6
    // 1

    double sum = 0;
    double nd = static_cast<double>(n);
    sum += (areaRel[0] * 0.5) / nd;
    for (size_t i = 1; i < n; i++)
    {
        sum += ((areaRel[i] + areaRel[i - 1]) * 0.5) / nd;
    }
#if defined(FORDIL_DEBUG_RESULT) || defined(FORDIL_DEBUG_RESULT_GINI)
    qDebug() << "sum" << sum;
#endif

    double result = (0.5 - sum) / 0.5;
#if defined(FORDIL_DEBUG_RESULT) || defined(FORDIL_DEBUG_RESULT_GINI)
    qDebug() << "Gini index" << result;
#endif

    return result;
}

static double resultTreeDistributionIndexShannon(const std::map<uint64_t, double> &map, double nSpecies)
{
    std::vector<double> areaSpecies;
    double areaTotal = 0;

    for (auto &[k, v] : map)
    {
        areaSpecies.push_back(v);
        areaTotal += v;
    }

    size_t n = areaSpecies.size();
    if (n < 2 || areaTotal < 1e-8)
    {
        return 0.0;
    }

    std::vector<double> areaP(n);
    for (size_t i = 0; i < n; i++)
    {
        areaP[i] = areaSpecies[i] / areaTotal;
    }

    std::vector<double> areaLn(n);
    double sumLn = 0;
    for (size_t i = 0; i < n; i++)
    {
        areaLn[i] = areaP[i] * std::log(areaP[i]);
        sumLn += areaLn[i];
    }

    double result = -sumLn / std::log(nSpecies);

#if defined(FORDIL_DEBUG_RESULT)
    qDebug() << "Shannon index" << result << " = -" << sumLn << "/"
             << std::log(nSpecies) << " : ln(" << nSpecies << ")";
#endif

    return result;
}

void Result::treeDistributionIndex(TreeDistributionIndex &result, Data &data)
{
    // Data
    std::vector<double> areaBefore;
    std::vector<double> areaAfter;

    std::map<uint64_t, double> areaSpeciesBefore;
    std::map<uint64_t, double> areaSpeciesAfter;

    // Iterate through all trees
    for (auto &[key, tree] : data.trees())
    {
        double gha = tree.gha();

        areaBefore.push_back(gha);
        areaSpeciesBefore[tree.speciesId()] += gha;

        if (!tree.isSelected())
        {
            areaAfter.push_back(gha);
            areaSpeciesAfter[tree.speciesId()] += gha;
        }
    }

    // Calculate Gini index before and after
    result.giniBefore = resultTreeDistributionIndexGini(areaBefore);
    result.giniAfter = resultTreeDistributionIndexGini(areaAfter);

    // Calculate Shannon index before and after
    double nSpecies = static_cast<double>(areaSpeciesBefore.size());
    result.shannonBefore = resultTreeDistributionIndexShannon(areaSpeciesBefore, nSpecies);
    result.shannonAfter = resultTreeDistributionIndexShannon(areaSpeciesAfter, nSpecies);
}

void Result::treeStatisticsSpecies(TreeStatistics &result,
                                   const Data &data,
                                   uint64_t speciesId,
                                   const QString &speciesFullName)
{
    // Initialize accumulators for cut and remaining trees
    double ghaBefore = 0;
    double volBefore = 0;
    double nBefore = 0;

    double ghaCut = 0;
    double volCut = 0;
    double nCut = 0;

    double ghaAfter = 0;
    double volAfter = 0;
    double nAfter = 0;

    double ghaBeforeTotal = 0;
    double volBeforeTotal = 0;
    double nBeforeTotal = 0;

    double ghaCutTotal = 0;
    double volCutTotal = 0;
    double nCutTotal = 0;

    double ghaAfterTotal = 0;
    double volAfterTotal = 0;
    double nAfterTotal = 0;

    // Iterate over all trees in the dataset and classify them
    // Accumulate GHA (basal area), volume, and count separately
    // depending on whether the tree is marked as 'selected' (i.e., to be cut)
    for (const auto &[key, tree] : data.trees())
    {
        // Skip processing this tree if its diameter at breast height (DBH)
        // is less than the minimum threshold defined by C_RESULT_DBH_MIN.
        if (tree.isDead() || tree.dbh() < C_RESULT_DBH_MIN)
        {
            continue; // Move to the next tree in the loop
        }

        double gha = tree.gha(); // GHA (ground area) of the current tree
        double vol = tree.vol(); // Volume of the current tree

        ghaBeforeTotal += gha;
        volBeforeTotal += vol;
        nBeforeTotal += 1.0;

        if (tree.isSelected())
        {
            // Tree is selected for cutting
            ghaCutTotal += gha;
            volCutTotal += vol;
            nCutTotal += 1.0;
        }
        else
        {
            // Tree remains after cutting
            ghaAfterTotal += gha;
            volAfterTotal += vol;
            nAfterTotal += 1.0;
        }

        // Skip processing if we are calculating another species
        if (speciesId != UINT64_MAX && speciesId != tree.speciesId())
        {
            continue; // Move to the next tree in the loop
        }

        ghaBefore += gha;
        volBefore += vol;
        nBefore += 1.0;

        if (tree.isSelected())
        {
            // Tree is selected for cutting
            ghaCut += gha;
            volCut += vol;
            nCut += 1.0;
        }
        else
        {
            // Tree remains after cutting
            ghaAfter += gha;
            volAfter += vol;
            nAfter += 1.0;
        }
    }

    // Calculate X/ha
    ghaBefore /= data.site().ha();
    volBefore /= data.site().ha();
    nBefore /= data.site().ha();

    ghaCut /= data.site().ha();
    volCut /= data.site().ha();
    nCut /= data.site().ha();

    ghaBeforeTotal /= data.site().ha();
    volBeforeTotal /= data.site().ha();
    nBeforeTotal /= data.site().ha();

    ghaCutTotal /= data.site().ha();
    volCutTotal /= data.site().ha();
    nCutTotal /= data.site().ha();

    // Percent
    double ghaBeforePercent = 100.0;
    double volBeforePercent = 100.0;
    double nBeforePercent = 100.0;

    double ghaCutPercent = ghaBefore > 1e-12 ? (ghaCut / ghaBefore) * 100.0 : 0.0;
    double volCutPercent = volBefore > 1e-12 ? (volCut / volBefore) * 100.0 : 0.0;
    double nCutPercent = nBefore > 1e-12 ? (nCut / nBefore) * 100.0 : 0.0;

    if (speciesId != UINT64_MAX)
    {
        ghaBeforePercent = ghaBeforeTotal > 1e-12 ? (ghaBefore / ghaBeforeTotal) * 100.0 : 0.0;
        volBeforePercent = volBeforeTotal > 1e-12 ? (volBefore / volBeforeTotal) * 100.0 : 0.0;
        nBeforePercent = nBeforeTotal > 1e-12 ? (nBefore / nBeforeTotal) * 100.0 : 0.0;

        ghaCutPercent = ghaBeforeTotal > 1e-12 ? (ghaCut / ghaBeforeTotal) * 100.0 : 0.0;
        volCutPercent = volBeforeTotal > 1e-12 ? (volCut / volBeforeTotal) * 100.0 : 0.0;
        nCutPercent = nBeforeTotal > 1e-12 ? (nCut / nBeforeTotal) * 100.0 : 0.0;
    }

    // Compute GHA, volume, and count after the trees were cut
    ghaAfter = ghaBefore - ghaCut;
    volAfter = volBefore - volCut;
    nAfter = nBefore - nCut;

    // Species
    TreeStatisticsSpecies species;

    species.fullName = speciesFullName;

    // Fill statistics for the 'Before' state (total values before any cutting)
    species.before.gha = ghaBefore;
    species.before.ghaPercent = ghaBeforePercent;
    species.before.volume = volBefore;
    species.before.volumePercent = volBeforePercent;
    species.before.n = nBefore;
    species.before.nPercent = nBeforePercent;

    // Fill statistics for the 'Cut' group (trees marked as selected for cutting)
    species.cut.gha = ghaCut;
    species.cut.ghaPercent = ghaCutPercent;
    species.cut.volume = volCut;
    species.cut.volumePercent = volCutPercent;
    species.cut.n = nCut;
    species.cut.nPercent = nCutPercent;

    // Fill statistics for the 'After' group (trees remaining after cutting)
    species.after.gha = ghaAfter;
    species.after.ghaPercent = ghaBeforePercent - ghaCutPercent;
    species.after.volume = volAfter;
    species.after.volumePercent = volBeforePercent - volCutPercent;
    species.after.n = nAfter;
    species.after.nPercent = nBeforePercent - nCutPercent;

    // Set result
    result.species.push_back(std::move(species));
}

void Result::treeStatistics(TreeStatistics &result, const Data &data, bool showSpecies)
{
    // Total statistics
    treeStatisticsSpecies(result, data, UINT64_MAX, "∑");

    // Statistics by species
    if (showSpecies)
    {
        std::map<QString, uint64_t> speciesMap;
        for (const auto &[key, tree] : data.trees())
        {
            const auto &species = data.species().at(tree.speciesId());
            speciesMap.insert({species.fullName(), tree.speciesId()});
        }

        for (const auto &[speciesName, speciesId] : speciesMap)
        {
            treeStatisticsSpecies(result, data, speciesId, speciesName);
        }
    }
}

void Result::ecologicalValues(ValuesList &result, const Data &data)
{
    // Clean
    result.values.clear();

    // Calculate
    Values prosilva;

    for (auto &[key, tree] : data.trees())
    {
        double value = 0;
        if (tree.q4Prosilvalue() > 0.0)
        {
            value = tree.ecovalueGlob();
        }

        prosilva.before += value;

        if (tree.isSelected())
        {
            prosilva.cut += value;
        }
    }

    prosilva.label = QCoreApplication::translate("Backend", "Prosilva");
    prosilva.after = prosilva.before - prosilva.cut;

    // Result
    result.values.push_back(std::move(prosilva));
}

void Result::ecologicalValuesDeadBio(ValuesList &result, const Data &data)
{
    // Clean
    result.values.clear();

    // Calculate
    Values dead;
    Values bio;

    for (auto &[key, tree] : data.trees())
    {
        if (tree.isDead())
        {
            dead.before += 1.0;
            if (tree.isSelected())
            {
                dead.cut += 1.0;
            }

        }

        if (tree.isBio())
        {
            bio.before += 1.0;
            if (tree.isSelected())
            {
                bio.cut += 1.0;
            }
        }
    }

    dead.label = QCoreApplication::translate("Backend", "Dead/ha");
    dead.before /= data.site().ha();
    dead.cut /= data.site().ha();
    dead.after = dead.before - dead.cut;

    bio.label = QCoreApplication::translate("Backend", "Bio/ha");
    bio.before /= data.site().ha();
    bio.cut /= data.site().ha();
    bio.after = bio.before - bio.cut;

    // Result
    result.values.push_back(std::move(dead));
    result.values.push_back(std::move(bio));
}

void Result::ecologicalValuesDmh(ValuesList &result, const Data &data)
{
    // Clean
    std::vector<Values> values;
    values.resize(7);
    for (size_t i = 0; i < values.size(); i++)
    {
        values[i] = Values();
    }

    values[0].label = QCoreApplication::translate("Backend", "Cavities");
    values[1].label = QCoreApplication::translate("Backend", "Dead wood in crown");
    values[2].label = QCoreApplication::translate("Backend", "Epiphytic, epixylic, parasitic");
    values[3].label = QCoreApplication::translate("Backend", "Mushrooms");
    values[4].label = QCoreApplication::translate("Backend", "Other ecological elements");
    values[5].label = QCoreApplication::translate("Backend", "Standing dead wood");
    values[6].label = QCoreApplication::translate("Backend", "Wounds and exposed wood");

    // Calculate
    for (auto &[key, tree] : data.trees())
    {
        const std::vector<int> &dmh = tree.dmh();
        if (dmh.size() < 7)
        {
            continue;
        }

        for (size_t i = 0; i < dmh.size(); i++)
        {
            if (dmh[i])
            {
                values[i].before += 1.0;
                if (tree.isSelected())
                {
                    values[i].cut += 1.0;
                }
            }
        }
    }

    // Update
    for (auto &it : values)
    {
        it.after = it.before - it.cut;

        if (it.before > 0.0)
        {
            it.afterPercent = (it.after / it.before) * 100.0;
            it.cutPercent = (it.cut / it.before) * 100.0;
        }
    }

    // 0 Reorder
    result.values.clear();
    result.values.push_back(values[0]);
    result.values.push_back(values[1]);
    result.values.push_back(values[2]);
    result.values.push_back(values[3]);
    result.values.push_back(values[6]);
    result.values.push_back(values[5]);
    result.values.push_back(values[4]);
}

static void resultEconomicValues(Result::ValuesList &result,
                                 const Data &data,
                                 bool user)
{
    // Clean
    result.values.clear();

    // Calculate
    Result::Values consumptionValue;
    Result::Values annualGain;
    Result::Values operatingRate;
    Result::Values potentialValue;

    const Typology &typology = data.typology();

    for (auto &[key, tree] : data.trees())
    {
        bool isTreeCut = false;
        if (user)
        {
            // User
            isTreeCut = tree.isSelected();
        }
        else
        {
            // Expert
            isTreeCut = typology.cutTreeIdsExpert().count(tree.treeId()) > 0;
        }

        consumptionValue.before += tree.cv();
        annualGain.before += tree.yGain();

        if (isTreeCut)
        {
            consumptionValue.cut += tree.cv();
            annualGain.cut += tree.yGain();
        }
    }

    consumptionValue.label = QCoreApplication::translate("Backend", "Consumption value");
    consumptionValue.before /= data.site().ha();
    consumptionValue.cut /= data.site().ha();
    consumptionValue.after = consumptionValue.before - consumptionValue.cut;

    annualGain.label = QCoreApplication::translate("Backend", "Annual gain");
    annualGain.before /= data.site().ha();
    annualGain.cut /= data.site().ha();
    annualGain.after = annualGain.before - annualGain.cut;

    operatingRate.label = QCoreApplication::translate("Backend", "Operating rate");
    if (consumptionValue.before > 0)
    {
        operatingRate.before = annualGain.before / consumptionValue.before;
    }

    if (consumptionValue.after > 0)
    {
        operatingRate.after = annualGain.after / consumptionValue.after;
    }

    potentialValue.label = QCoreApplication::translate("Backend", "Potential value");
    potentialValue.before = annualGain.before / 0.02;
    potentialValue.cut = annualGain.cut / 0.02;
    potentialValue.after = potentialValue.before - potentialValue.cut;

    // Result
    result.values.push_back(std::move(consumptionValue));
    result.values.push_back(std::move(annualGain));
    // result.values.push_back(std::move(operatingRate)); ignore operating rate
    result.values.push_back(std::move(potentialValue));
}

void Result::economicValuesUser(ValuesList &result, const Data &data)
{
    resultEconomicValues(result, data, true);
}

void Result::economicValuesExpert(ValuesList &result, const Data &data)
{
    resultEconomicValues(result, data, false);
}

static void resultQualitySumVol(size_t &count,
                                double &sum,
                                const QString &qual,
                                Data &data,
                                bool onlySelected = false)
{
    count = 0;
    sum = 0;

    for (const auto &[key, tree] : data.trees())
    {
        if (tree.isDead())
        {
            continue;
        }

        if ((!onlySelected) || (onlySelected && tree.isSelected()))
        {
            double vol = tree.vol(qual);
            if (vol > 0.0)
            {
                count++;
                sum += vol;
            }
        }
    }
}

static void resultFixTo100(std::vector<Result::Slice> &slices)
{
    size_t maxLabelIndex = SIZE_MAX;
    double maxPercent = 0;

    for (size_t i = 0; i < slices.size(); i++)
    {
        if (slices[i].value > maxPercent)
        {
            maxPercent = slices[i].value;
            maxLabelIndex = i;
        }
    }

    // fix to 100.0% total
    if (maxLabelIndex != SIZE_MAX && maxPercent > 0.0)
    {
        double sum = 0;
        for (size_t i = 0; i < slices.size(); i++)
        {
            if (i != maxLabelIndex)
            {
                sum += slices[i].value;
            }
        }

        double percent = 100.0 - sum;
        slices[maxLabelIndex].value = percent;
    }
}

static void resultQuality(const QString &component,
                          std::vector<Result::Slice> &slices,
                          const std::vector<QString> &labels,
                          const std::vector<QString> &colors,
                          const std::vector<size_t> &count,
                          const std::vector<double> &sum,
                          const std::vector<double> &sumPerLabel,
                          double sumTotal)
{
    slices.clear();

    for (size_t i = 0; i < labels.size(); i++)
    {
        double percent = 0;

        if (sumTotal > 0.0)
        {
            percent = (sum[i] / sumTotal) * 100.0;
        }

        slices.push_back({labels[i], percent, colors[i]});
    }

    resultFixTo100(slices);
}

void Result::quality(SlicesCut &result, Data &data)
{
    std::vector<QString> labels = {"A", "B", "C", "D", "FW"};
    std::vector<QString> colors{"#a8eabc", "#a4b8d3", "#fede8a", "#f7a3a1", "#caa8e8"};

    size_t nLabels = labels.size();

    std::vector<size_t> countBefore(nLabels);
    std::vector<double> sumBefore(nLabels);
    double sumBeforeTotal = 0;

    std::vector<size_t> countCut(nLabels);
    std::vector<double> sumCut(nLabels);
    double sumCutTotal = 0;

    std::vector<size_t> countAfter(nLabels);
    std::vector<double> sumAfter(nLabels);
    double sumAfterTotal = 0;

    for (size_t i = 0; i < nLabels; i++)
    {
        resultQualitySumVol(countBefore[i], sumBefore[i], labels[i], data);
        resultQualitySumVol(countCut[i], sumCut[i], labels[i], data, true);

        countAfter[i] = countBefore[i] - countCut[i];
        sumAfter[i] = sumBefore[i] - sumCut[i];

        sumBeforeTotal += sumBefore[i];
        sumCutTotal += sumCut[i];
        sumAfterTotal += sumAfter[i];

#if defined(FORDIL_DEBUG_RESULT)
        qDebug() << "quality" << labels[i]
                 << "before count" << countBefore[i] << "sum" << sumBefore[i]
                 << ", cut count" << countCut[i] << "sum" << sumCut[i]
                 << ", after count" << countAfter[i] << "sum" << sumAfter[i];
#endif
    }

#if defined(FORDIL_DEBUG_RESULT)
    qDebug() << "quality total sum before" << sumBeforeTotal << "cut" << sumCutTotal << "after" << sumAfterTotal;
#endif

    resultQuality("Before", result.before, labels, colors, countBefore, sumBefore, sumBefore, sumBeforeTotal);
    resultQuality("Fell",    result.cut,    labels, colors, countCut,    sumCut,    sumCut,    sumCutTotal);
    resultQuality("After",  result.after,  labels, colors, countAfter,  sumAfter,  sumBefore, sumAfterTotal);
}

static double resultQualityEvolutionValue(const std::vector<Result::Slice> &slices, const QString &label)
{
    for (size_t i = 0; i < slices.size(); i++)
    {
        if (slices[i].label == label)
        {
            return slices[i].value;
        }
    }

    return 0.0;
}

double Result::qualityEvolution(Data &data)
{
    Result::SlicesCut slices;
    
    Result::quality(slices, data);

    // Before
    double aBefore = resultQualityEvolutionValue(slices.before, "A");
    double bBefore = resultQualityEvolutionValue(slices.before, "B");
    double cBefore = resultQualityEvolutionValue(slices.before, "C");

    double abcBefore = aBefore + bBefore + cBefore;

    // After
    double aAfter = resultQualityEvolutionValue(slices.after, "A");
    double bAfter = resultQualityEvolutionValue(slices.after, "B");
    double cAfter = resultQualityEvolutionValue(slices.after, "C");

    double abcAfter = aAfter + bAfter + cAfter;

    // Calculate
    double before = abcBefore;
    double after = abcAfter;
    double diff = after - before;

    double result = 0; // same
    if (diff > 2.0)
    {
        result = 1.0; // better        
    }
    else if (diff < -2.0)
    {
        result = -1.0; // worse
    }

#if defined(FORDIL_DEBUG_RESULT)
    qDebug() << "before A" << aBefore << "% + B" << bBefore << "% + C" << cBefore << "% = ABC" << abcBefore << "%";
    qDebug() << "before" << before << "%";

    qDebug() << "after A" << aAfter << "% + B" << bAfter << "% + C" << cAfter << "% = ABC" << abcAfter << "%";
    qDebug() << "after" << after << "%";

    qDebug() << "after" << after << "- before" << before << "= diff" << diff << "-> result" << result;
#endif

    return result;
}

static void resultComposition(std::vector<Result::Slice> &slices,
                              size_t when,
                              const std::map<QString, std::array<double, 3>> &species,
                              const std::map<QString, QColor> &speciesColor)
{
    slices.clear();

    double sumTotal = 0;
    for (const auto &[k, v] : species)
    {
        sumTotal += v[when];
    }

    for (const auto &[k, v] : species)
    {
        double percent = 0;

        if (sumTotal > 0.0)
        {
            percent = (v[when] / sumTotal) * 100.0;
        }

        slices.push_back({k, percent, speciesColor.at(k)});
    }

    resultFixTo100(slices);
}

void Result::composition(SlicesCut &result, Data &data)
{
    result.before.clear();
    result.cut.clear();
    result.after.clear();

    std::map<QString, std::array<double, 3>> species;
    std::map<QString, QColor> speciesColor;
    for (const auto &[key, tree] : data.trees())
    {
        if (tree.isDead())
        {
            continue;
        }

        auto itName = data.species().find(tree.speciesId());
        if (itName == data.species().end())
        {
            continue;
        }

        std::array<double, 3> item;
        item[0] = tree.vol();
        if (tree.isSelected())
        {
            item[1] = item[0];
            item[2] = 0.0;
        }
        else
        {
            item[1] = 0.0;
            item[2] = item[0];
        }

        auto it = species.find(itName->second.fullName());
        if (it != species.end())
        {
            it->second[0] += item[0];
            it->second[1] += item[1];
            it->second[2] += item[2];
        }
        else
        {
            species[itName->second.fullName()] = item;
            speciesColor[itName->second.fullName()] = itName->second.color();
        }
    }

    //         full name         Before, Cut, After
    //                               [0]  [1]    [2]
    // species["Abies alba"] = {      4,   0,     4 }
    // species["Cedrus sp"]  = {      3,   1,     2 }

    resultComposition(result.before, 0, species, speciesColor);
    resultComposition(result.cut, 1, species, speciesColor);
    resultComposition(result.after, 2, species, speciesColor);
}

void Result::choicesAssessment(std::vector<Result::Slice> &result, Data &data)
{
    // Calculate the number of categories needed
    size_t n = static_cast<size_t>(TreeSelectionReasonValue::End);
#if defined(FORDIL_DEBUG_RESULT)
    qDebug() << "n" << n;
#endif

    // Resize the items array to hold all categories
    std::vector<Result::Slice> items;
    items.resize(n);

    // Initialize and calculate the result items
    double valueTotal = 0;
    for (size_t i = 0; i < n; i++)
    {
        TreeSelectionReasonValue choice = static_cast<TreeSelectionReasonValue>(i);
        items[i].label = treeSelectionReasonToStringTr(choice);
        items[i].color = treeSelectionReasonColor(choice);
        items[i].value = 0;

        for (auto &[key, tree] : data.trees())
        {
            if (tree.treeSelectionChoices().checked(choice))
            {
                items[i].value += 1.0;
                valueTotal += 1.0;
            }
        }
    }

    // To percents
    if (valueTotal > 0.0)
    {
        for (size_t i = 0; i < n; i++)
        {
            items[i].value = (items[i].value / valueTotal) * 100.0;
        }
    }

    // Use a subset with cutting reasons
    result.clear();
    for (size_t i = 0; i < n; i++)
    {
        TreeSelectionReasonValue choice = static_cast<TreeSelectionReasonValue>(i);
        if (choice > TreeSelectionReasonValue::None &&
            choice < TreeSelectionReasonValue::Siblings)
        {
            result.push_back(items[i]);
        }
    }

    resultFixTo100(result);
}

static void resultProcessValuationResult(Result::ValuationResult &result)
{
    QString colorGood{"#bbedca"};
    QString colorAverage{"#fbedbc"};
    QString colorBad{"#f9d2d7"};

    if (result.total > 0)
    {
        result.mark = static_cast<double>(result.score) / static_cast<double>(result.total);
        if (result.mark > 0.0)
        {
            if (result.mark < 1.0)
            {
                result.color = colorAverage;
            }
            else
            {
                result.color = colorGood;
            }
        }
        else
        {
            result.color = colorBad;
        }
    }
    else
    {
        result.score = 0;
        result.total = 0;
        result.mark = 0.0;
        result.color = colorGood;
    }

    result.text = QString::number(result.score) + "/" + QString::number(result.total);
}

static void resultSortValuation(Result::Valuation &result)
{
    std::sort(result.reasons.begin(), result.reasons.end(),
              [](const Result::ValuationResult& a,
                 const Result::ValuationResult& b)
    {
        return a.score > b.score || a.total > b.total;
    });
}

void Result::hotspots(Valuation &valuation, const Data &data)
{
    // Clear
    valuation.ranking = ValuationResult();
    valuation.reasons.clear();

    // Calculate results
    std::map<TreeSelectionReasonValue, ValuationResult> resultMap;
    for (const auto &[key, hotspot] : data.hotspots())
    {
        TreeSelectionReasonValue hotspotReason = hotspot.reason();
        auto it = resultMap.insert(std::make_pair(hotspotReason, ValuationResult()));
        ValuationResult &hotspotResult = it.first->second;

        bool correctHotspotCut = true;
        std::map<uint64_t, bool> cutResults = hotspot.cutResults(data.trees());
        for (const auto &it : cutResults)
        {
            if (!it.second)
            {
                correctHotspotCut = false;
                break;
            }
        }

        hotspotResult.treeSelectionReason = hotspotReason;
        hotspotResult.total++;

        if (correctHotspotCut)
        {
            hotspotResult.score++;
        }
    }

    // Add results
    for (auto &[key, hotspotResult] : resultMap)
    {
        resultProcessValuationResult(hotspotResult);
        valuation.reasons.push_back(hotspotResult);
    }

    // Sort results in descending order
    resultSortValuation(valuation);

    // Ranking
    for (const auto &it : valuation.reasons)
    {
        valuation.ranking.score += it.score;
        valuation.ranking.total += it.total;
    }

    resultProcessValuationResult(valuation.ranking);
}

