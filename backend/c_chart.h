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

#ifndef C_CHART_H
#define C_CHART_H

#include <QPainter>
#include "c_data.h"

class Chart
{
public:
    class Curve
    {
    public:
        QString id;
        QString name;
        QColor color;
        bool enabled{false};
        qreal a{-1.0};
        qreal k{-1.0};
    };

    class Options
    {
    public:
        QString treeDistributionType;
        std::vector<Curve> curves;

        bool showAllTreeStatisticsSpecies{false};

        QString typologyLocalType;
        QString typologyGlobalAge;
    };

    class Cell
    {
    public:
        QString text;
        QColor textColor;
        QColor color;
        bool rowSpan{false};
        bool italic{false};
    };

    class Table
    {
    public:
        std::vector<std::vector<Cell>> rows;
    };

    Chart();

    // Any
    static void requestImage(QPainter &painter,
                             const QString &imageId,
                             Data &data,
                             const Chart::Options &options);

    static double imageHeight(double width,
                              const QString &imageId,
                              Data &data,
                              const Options &options);

    // Plots
    static void drawTreeDistribution(QPainter &painter, Data &data, const Options &options);
    static void drawQuality(QPainter &painter, const QString &type, Data &data);
    static void drawQualityEvolution(QPainter &painter, Data &data);
    static void drawComposition(QPainter &painter, const QString &type, Data &data);
    static void drawChoicesAssessment(QPainter &painter, Data &data);
    static void drawEcologicalValuesDmh(QPainter &painter, Data &data);

    // Tables
    static void drawTreeStatistics(QPainter &painter, Data &data, const Options &options);
    static void drawEconomicUserValues(QPainter &painter, Data &data);
    static void drawEconomicExpertValues(QPainter &painter, Data &data);
    static void drawGiniIndex(QPainter &painter, Data &data);
    static void drawThinningFactor(QPainter &painter, Data &data);
    static void drawShannonIndex(QPainter &painter, Data &data);
    static void drawEcologicalValues(QPainter &painter, Data &data);
    static void drawEcologicalValuesDeadBio(QPainter &painter, Data &data);

    // Typology
    static void drawTypologyLocal(QPainter &painter, const Data &data, const Options &options);
    static void drawTypologyLocal(QPainter &painter, const Data &data, const QString &when, const Options &options);
    static void drawTypologyGlobal(QPainter &painter, const Data &data, const Options &options);
    static void drawOverallAssessment(QPainter &painter, const Data &data);

    // Valuation
    static QVariantList hotspotsModelData(const Data &data);
};

#endif // C_CHART_H
