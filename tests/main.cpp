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

#include <QCoreApplication>
#include <QDebug>
#include "test.h"
#include "../backend/c_data.h"
#include "../backend/c_result.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    int rc = -1;

    try
    {
        Test::instance()->run();
        std::cout << "Tests Failed: " << Test::instance()->failed() << std::endl;
        if (Test::instance()->failed() == 0)
        {
            rc = 0;
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
    }

    return rc;
}

#define ENABLE_LOG
#ifdef ENABLE_LOG
    #define LOG_TEST qDebug()
#else
    #define LOG_TEST while (false) qDebug()
#endif

static bool testEqual(double a, double b, int decimals = 1)
{
    QString strA = QString::number(a, 'f', decimals);
    QString strB = QString::number(b, 'f', decimals);

    return strA == strB;
}

static QString testLoadSite(Data &data,
                            const QString &filePath,
                            bool discovery = false)
{
    QString system;
    QString siteName;
    QString error;

    data.inspectDataFile(filePath, system, siteName, error);
    LOG_TEST << "system" << system
             << "siteName" << siteName
             << "error" << error;

    if (!error.isEmpty())
    {
        return error;
    }

    data.loadExercise(filePath, siteName, system, discovery);

    return error;
}

TEST_CASE(TestResultsPokojna)
{
    Data data;
    QString error;

    error = testLoadSite(data, "data/Marteloscope_Pokojna_CZ.xlsx");

    TEST(error.isEmpty());
    TEST(data.trees().size() == 357);

    size_t n;

    // treeStatistics
    Result::TreeStatistics resultTS;
    Result::treeStatistics(resultTS, data, false);
    n = resultTS.species.size();
    TEST(n > 0);
    TEST(n > 0 && testEqual(resultTS.species[0].before.gha,     30.7));
    TEST(n > 0 && testEqual(resultTS.species[0].before.volume, 415.9));
    TEST(n > 0 && testEqual(resultTS.species[0].before.n,      342.2));

    // economicValuesUser
    Result::ValuesList resultEU;
    Result::economicValuesUser(resultEU, data);
    n = resultEU.values.size();
    TEST(n == 3);
    TEST(n == 3 && testEqual(resultEU.values[0].before, 1044241.4));
    TEST(n == 3 && testEqual(resultEU.values[1].before,   35592.3));
    TEST(n == 3 && testEqual(resultEU.values[2].before, 1779613.6));

    // economicValuesExpert
    Result::ValuesList resultEE;
    Result::economicValuesExpert(resultEE, data);
    n = resultEE.values.size();
    TEST(n == 3);
    TEST(n == 3 && testEqual(resultEE.values[0].before, 1044241.4));
    TEST(n == 3 && testEqual(resultEE.values[1].before,   35592.3));
    TEST(n == 3 && testEqual(resultEE.values[2].before, 1779613.6));
    TEST(n == 3 && testEqual(resultEE.values[0].cut,     316839.0));
    TEST(n == 3 && testEqual(resultEE.values[1].cut,      10018.1));
    TEST(n == 3 && testEqual(resultEE.values[2].cut,     500903.5));
    TEST(n == 3 && testEqual(resultEE.values[0].after,   727402.4));
    TEST(n == 3 && testEqual(resultEE.values[1].after,    25574.2));
    TEST(n == 3 && testEqual(resultEE.values[2].after,  1278710.1));

    // treeDistributionIndex
    Result::TreeDistributionIndex resultTDI;
    Result::treeDistributionIndex(resultTDI, data);
    TEST(testEqual(resultTDI.giniBefore,    0.47, 2));
    TEST(testEqual(resultTDI.giniAfter,     0.47, 2));
    TEST(testEqual(resultTDI.shannonBefore, 0.73, 2));
    TEST(testEqual(resultTDI.shannonAfter,  0.73, 2));

    // thinningFactor
    TEST(std::isnan(Result::thinningFactor(data)));

    // quality
    Result::SlicesCut resultQual;
    Result::quality(resultQual, data);
    n = resultQual.before.size();
    TEST(n == 5);
    TEST(n == 5 && testEqual(resultQual.before[0].value, 20.2)); // A
    TEST(n == 5 && testEqual(resultQual.before[1].value, 24.3)); // B
    TEST(n == 5 && testEqual(resultQual.before[2].value, 34.6)); // C
    TEST(n == 5 && testEqual(resultQual.before[3].value,  0.0)); // D
    TEST(n == 5 && testEqual(resultQual.before[4].value, 20.9)); // FW

    // composition
    Result::SlicesCut resultComp;
    Result::composition(resultComp, data);
    n = resultComp.before.size();
    TEST(n == 5);
    TEST(n == 5 && testEqual(resultComp.before[0].value, 10.7));
    TEST(n == 5 && testEqual(resultComp.before[1].value, 65.4));
    TEST(n == 5 && testEqual(resultComp.before[2].value, 15.2));
    TEST(n == 5 && testEqual(resultComp.before[3].value,  5.1));
    TEST(n == 5 && testEqual(resultComp.before[4].value,  3.6));

    // ecologicalValuesDeadBio
    Result::ValuesList resultEDB;
    Result::ecologicalValuesDeadBio(resultEDB, data);
    n = resultEDB.values.size();
    TEST(n == 2);
    TEST(n == 2 && testEqual(resultEDB.values[0].before,  1.0));
    TEST(n == 2 && testEqual(resultEDB.values[1].before, 19.2));

    // ecologicalValues
    Result::ValuesList resultEP;
    Result::ecologicalValues(resultEP, data);
    n = resultEP.values.size();
    TEST(n == 1);
    TEST(n == 1 && testEqual(resultEP.values[0].before, 1655.5));
}

TEST_CASE(TestResultsLauzelle)
{
    Data data;
    QString error;

    error = testLoadSite(data, "data/Marteloscope_Lauzelle_BE.xlsx");

    TEST(error.isEmpty());
    TEST(data.trees().size() == 288);

    // cut trees 26, 28 (competition), 107 (maturity)
    TreeSelectionChoices selectionChoicesC(true, false, false, false,
                                           false, false, false);
    data.trees().at(26).confirmSelection(selectionChoicesC);
    data.trees().at(28).confirmSelection(selectionChoicesC);

    TreeSelectionChoices selectionChoicesM(false, true, false, false,
                                           false, false, false);
    data.trees().at(107).confirmSelection(selectionChoicesM);

    // n
    size_t n;

    // treeStatistics
    Result::TreeStatistics resultTS;
    Result::treeStatistics(resultTS, data, false);
    n = resultTS.species.size();
    TEST(n > 0);
    TEST(n > 0 && testEqual(resultTS.species[0].before.gha,     24.5));
    TEST(n > 0 && testEqual(resultTS.species[0].before.volume, 333.4));
    TEST(n > 0 && testEqual(resultTS.species[0].before.n,      167.6));
    TEST(n > 0 && testEqual(resultTS.species[0].cut.gha,         0.9));
    TEST(n > 0 && testEqual(resultTS.species[0].cut.volume,     12.1));
    TEST(n > 0 && testEqual(resultTS.species[0].cut.n,           1.8));
    TEST(n > 0 && testEqual(resultTS.species[0].after.gha,      23.6));
    TEST(n > 0 && testEqual(resultTS.species[0].after.volume,  321.3));
    TEST(n > 0 && testEqual(resultTS.species[0].after.n,       165.8));

    // economicValuesUser
    Result::ValuesList resultEU;
    Result::economicValuesUser(resultEU, data);
    n = resultEU.values.size();
    TEST(n == 3);
    TEST(n == 3 && testEqual(resultEU.values[0].before, 29368.3));
    TEST(n == 3 && testEqual(resultEU.values[1].before,   581.2));
    TEST(n == 3 && testEqual(resultEU.values[2].before, 29059.5));
    TEST(n == 3 && testEqual(resultEU.values[0].cut,     2201.0));
    TEST(n == 3 && testEqual(resultEU.values[1].cut,       31.8));
    TEST(n == 3 && testEqual(resultEU.values[2].cut,     1588.0));
    TEST(n == 3 && testEqual(resultEU.values[0].after,  27167.3));
    TEST(n == 3 && testEqual(resultEU.values[1].after,    549.4));
    TEST(n == 3 && testEqual(resultEU.values[2].after,  27471.4));

    // economicValuesExpert
    Result::ValuesList resultEE;
    Result::economicValuesExpert(resultEE, data);
    n = resultEE.values.size();
    TEST(n == 3);
    TEST(n == 3 && testEqual(resultEE.values[0].before, 29368.3));
    TEST(n == 3 && testEqual(resultEE.values[1].before,   581.2));
    TEST(n == 3 && testEqual(resultEE.values[2].before, 29059.5));
    TEST(n == 3 && testEqual(resultEE.values[0].cut,     4088.5));
    TEST(n == 3 && testEqual(resultEE.values[1].cut,      129.9));
    TEST(n == 3 && testEqual(resultEE.values[2].cut,     6492.8));
    TEST(n == 3 && testEqual(resultEE.values[0].after,  25279.8));
    TEST(n == 3 && testEqual(resultEE.values[1].after,    451.3));
    TEST(n == 3 && testEqual(resultEE.values[2].after,  22566.7));

    // treeDistributionIndex
    Result::TreeDistributionIndex resultTDI;
    Result::treeDistributionIndex(resultTDI, data);
    TEST(testEqual(resultTDI.giniBefore,    0.53, 2));
    TEST(testEqual(resultTDI.giniAfter,     0.53, 2));
    TEST(testEqual(resultTDI.shannonBefore, 0.69, 2));
    TEST(testEqual(resultTDI.shannonAfter,  0.70, 2));

    // thinningFactor
    TEST(testEqual(Result::thinningFactor(data), 2.08, 2));

    // quality
    Result::SlicesCut resultQual;
    Result::quality(resultQual, data);
    n = resultQual.before.size();
    TEST(n == 5);
    TEST(n == 5 && testEqual(resultQual.before[0].value,  0.0)); // A
    TEST(n == 5 && testEqual(resultQual.before[1].value,  3.7)); // B
    TEST(n == 5 && testEqual(resultQual.before[2].value, 15.5)); // C
    TEST(n == 5 && testEqual(resultQual.before[3].value, 38.9)); // D
    TEST(n == 5 && testEqual(resultQual.before[4].value, 41.8)); // FW

    // cut
    n = resultQual.cut.size();
    TEST(n == 5);
    TEST(n == 5 && testEqual(resultQual.cut[0].value,  0.0)); // A
    TEST(n == 5 && testEqual(resultQual.cut[1].value,  6.9)); // B
    TEST(n == 5 && testEqual(resultQual.cut[2].value, 25.3)); // C
    TEST(n == 5 && testEqual(resultQual.cut[3].value, 34.5)); // D
    TEST(n == 5 && testEqual(resultQual.cut[4].value, 33.3)); // FW

    // after
    n = resultQual.after.size();
    TEST(n == 5);
    TEST(n == 5 && testEqual(resultQual.after[0].value,  0.0)); // A
    TEST(n == 5 && testEqual(resultQual.after[1].value,  3.6)); // B
    TEST(n == 5 && testEqual(resultQual.after[2].value, 15.1)); // C
    TEST(n == 5 && testEqual(resultQual.after[3].value, 39.1)); // D
    TEST(n == 5 && testEqual(resultQual.after[4].value, 42.2)); // FW

    // composition
    Result::SlicesCut resultComp;
    Result::composition(resultComp, data);
    n = resultComp.before.size();
    TEST(n == 9);
    TEST(n == 9 && testEqual(resultComp.before[0].value, 25.9)); // Acer
    TEST(n == 9 && testEqual(resultComp.before[1].value,  0.3)); // Betula
    TEST(n == 9 && testEqual(resultComp.before[2].value,  6.3)); // Carpinus
    TEST(n == 9 && testEqual(resultComp.before[3].value,  3.2)); // Fagus
    TEST(n == 9 && testEqual(resultComp.before[4].value,  5.4)); // Fraxinus
    TEST(n == 9 && testEqual(resultComp.before[5].value,  1.4)); // Picea sp
    TEST(n == 9 && testEqual(resultComp.before[6].value,  4.1)); // Populus sp
    TEST(n == 9 && testEqual(resultComp.before[7].value,  6.1)); // Prunus sp
    TEST(n == 9 && testEqual(resultComp.before[8].value, 47.5)); // Quercus

    // cut
    // Quercus 100.0
    n = resultComp.cut.size();
    TEST(n == 9);
    TEST(n == 9 && testEqual(resultComp.cut[8].value, 100.0)); // Quercus

    // after
    TEST(n == 9 && testEqual(resultComp.after[0].value, 26.9));
    TEST(n == 9 && testEqual(resultComp.after[1].value,  0.3));
    TEST(n == 9 && testEqual(resultComp.after[2].value,  6.5));
    TEST(n == 9 && testEqual(resultComp.after[3].value,  3.3));
    TEST(n == 9 && testEqual(resultComp.after[4].value,  5.6));
    TEST(n == 9 && testEqual(resultComp.after[5].value,  1.4));
    TEST(n == 9 && testEqual(resultComp.after[6].value,  4.2));
    TEST(n == 9 && testEqual(resultComp.after[7].value,  6.3));
    TEST(n == 9 && testEqual(resultComp.after[8].value, 45.5));

    // choicesAssessment
    std::vector<Result::Slice> slicesCHA;
    Result::choicesAssessment(slicesCHA, data);
    n = slicesCHA.size();
    TEST(n > 1);
    TEST(n > 1 && testEqual(slicesCHA[0].value, 66.7)); // Competition
    TEST(n > 1 && testEqual(slicesCHA[1].value, 33.3)); // Maturity

    // ecologicalValuesDeadBio
    Result::ValuesList resultEDB;
    Result::ecologicalValuesDeadBio(resultEDB, data);
    n = resultEDB.values.size();
    TEST(n == 2);
    TEST(n == 2 && testEqual(resultEDB.values[0].before, 1.2)); // Dead/ha
    TEST(n == 2 && testEqual(resultEDB.values[1].before, 7.0)); // Bio/ha

    // ecologicalValues
    Result::ValuesList resultEP;
    Result::ecologicalValues(resultEP, data);
    n = resultEP.values.size();
    TEST(n == 1);
    TEST(n == 1 && testEqual(resultEP.values[0].before, 3482.5)); // Prosilva 
    TEST(n == 1 && testEqual(resultEP.values[0].cut,     110.5)); // Prosilva
    TEST(n == 1 && testEqual(resultEP.values[0].after,  3372.0)); // Prosilva
}

TEST_CASE(TestResultsMormal)
{
    Data data;
    QString error;

    error = testLoadSite(data, "data/Marteloscope_Mormal_FR.xlsx");

    TEST(error.isEmpty());
    TEST(data.trees().size() == 359);

    size_t n;

    // treeStatistics
    Result::TreeStatistics resultTS;
    Result::treeStatistics(resultTS, data, false);
    n = resultTS.species.size();
    TEST(n > 0);
    TEST(n > 0 && testEqual(resultTS.species[0].before.gha,     22.9));
    TEST(n > 0 && testEqual(resultTS.species[0].before.volume, 265.6));
    TEST(n > 0 && testEqual(resultTS.species[0].before.n,      245.9));

    // economicValuesUser
    Result::ValuesList resultEU;
    Result::economicValuesUser(resultEU, data);
    n = resultEU.values.size();
    TEST(n == 3);
    TEST(n == 3 && testEqual(resultEU.values[0].before, 20989.8));
    TEST(n == 3 && testEqual(resultEU.values[1].before,   562.8));
    TEST(n == 3 && testEqual(resultEU.values[2].before, 28137.8));

    // economicValuesExpert
    Result::ValuesList resultEE;
    Result::economicValuesExpert(resultEE, data);
    n = resultEE.values.size();
    TEST(n == 3);
    TEST(n == 3 && testEqual(resultEE.values[0].before, 20989.8));
    TEST(n == 3 && testEqual(resultEE.values[1].before,   562.8));
    TEST(n == 3 && testEqual(resultEE.values[2].before, 28137.8));
    TEST(n == 3 && testEqual(resultEE.values[0].cut,     2006.7));
    TEST(n == 3 && testEqual(resultEE.values[1].cut,       45.7));
    TEST(n == 3 && testEqual(resultEE.values[2].cut,     2285.2));
    TEST(n == 3 && testEqual(resultEE.values[0].after,  18983.1));
    TEST(n == 3 && testEqual(resultEE.values[1].after,    517.1));
    TEST(n == 3 && testEqual(resultEE.values[2].after,  25852.6));
}
