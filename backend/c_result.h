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

#ifndef C_RESULT_H
#define C_RESULT_H

#include <QColor>
#include <QString>
#include <QVariant>
#include <vector>
#include <map>
#include "c_data.h"

class Result
{
public:
    Result();

    // Stacked Bar
    class StackedBarItem
    {
    public:
        QString category;
        std::vector<double> values;
    };

    class StackedBarSet
    {
    public:
        QString label;
        QColor color;
    };

    class StackedBarChart
    {
    public:
        std::vector<StackedBarItem> items;
        std::vector<StackedBarSet> sets;
        QString xLabel;
        QString yLabel;
    };

    static void treesDistribution(StackedBarChart &result,
                                  Data &data,
                                  const QString &distributionOption);


    // Tree distribution index
    static double thinningFactor(Data &data);

    class TreeDistributionIndex
    {
    public:
        double giniBefore{0.0};
        double giniAfter{0.0};

        double shannonBefore{0.0};
        double shannonAfter{0.0};
    };

    static void treeDistributionIndex(TreeDistributionIndex &result, Data &data);

    // Tree Statistics
    class TreeStatisticsComponent
    {
    public:
        double gha{0.0};
        double ghaPercent{0.0};

        double volume{0.0};
        double volumePercent{0.0};

        double n{0.0};
        double nPercent{0.0};
    };

    class TreeStatisticsSpecies
    {
    public:
        QString fullName;
        TreeStatisticsComponent before;
        TreeStatisticsComponent cut;
        TreeStatisticsComponent after;
    };

    class TreeStatistics
    {
    public:
        std::vector<TreeStatisticsSpecies> species;
    };

    static void treeStatisticsSpecies(TreeStatistics &result,
                                      const Data &data,
                                      uint64_t speciesId = UINT64_MAX,
                                      const QString &speciesFullName = "");

    static void treeStatistics(TreeStatistics &result,
                               const Data &data,
                               bool showSpecies);

    // Ecological Values
    class Values
    {
    public:
        QString label;
        double before{0.0};
        double cut{0.0};
        double cutPercent{0.0};
        double after{0.0};
        double afterPercent{0.0};
    };

    class ValuesList
    {
    public:
        std::vector<Values> values;
    };

    // Ecological Values
    static void ecologicalValues(ValuesList &result, const Data &data);

    // Ecological Values : Dead/Bio
    static void ecologicalValuesDeadBio(ValuesList &result, const Data &data);

    // Ecological Values : DMH dendro-micro-habitat
    static void ecologicalValuesDmh(ValuesList &result, const Data &data);

    // Economic Values
    static void economicValuesUser(ValuesList &result, const Data &data);
    static void economicValuesExpert(ValuesList &result, const Data &data);

    // Quality
    class Slice
    {
    public:
        QString label;
        double value{0.0};
        QColor color;
    };

    class SlicesCut
    {
    public:
        std::vector<Slice> before;
        std::vector<Slice> cut;
        std::vector<Slice> after;
    };

    static void quality(SlicesCut &result, Data &data);
    static double qualityEvolution(Data &data);

    static void composition(SlicesCut &result, Data &data);

    static void choicesAssessment(std::vector<Result::Slice> &result, Data &data);

    // Valuation
    class ValuationResult
    {
    public:
        TreeSelectionReasonValue treeSelectionReason{TreeSelectionReasonValue::None};
        QString reasonId;
        QString reasonTextTr;
        int total{0};
        int score{0};
        double mark{1.0};
        QString text;
        QString color;
    };

    class Valuation
    {
    public:
        ValuationResult ranking;
        std::vector<ValuationResult> reasons;
    };

    static void hotspots(Valuation &valuation, const Data &data);
};

#endif // C_RESULT_H
