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

#include "c_chart.h"
#include "c_result.h"
#include "c_mappainter.h"
#include <QCoreApplication>
#include <QDebug>

Chart::Chart()
{
    // empty
}

static qreal chartNiceNumber(qreal value, bool round)
{
    qreal exponent = std::floor(std::log10(value));
    qreal fraction = value / std::pow(10, exponent);

    qreal niceFraction;
    if (round) {
        if (fraction < 1.5)
            niceFraction = 1;
        else if (fraction < 3)
            niceFraction = 2;
        else if (fraction < 7)
            niceFraction = 5;
        else
            niceFraction = 10;
    } else {
        if (fraction <= 1)
            niceFraction = 1;
        else if (fraction <= 2)
            niceFraction = 2;
        else if (fraction <= 5)
            niceFraction = 5;
        else
            niceFraction = 10;
    }

    return niceFraction * std::pow(10, exponent);
}

static qreal chartNiceStep(qreal maxY, int tickCount)
{
    qreal rawStep = maxY / (tickCount - 1);
    return chartNiceNumber(rawStep, true);  // `true` means round to nearest nice value
}

static QString chartNumber(double value)
{
    // Convert the double to a string with 1 digits after the decimal point
    QString str = QString::number(value, 'f', 1);

    // Trim trailing zeros
    while (str.endsWith('0'))
    {
        str.chop(1);
    }

    // If it ends with a '.', remove that too (e.g. "10.")
    if (str.endsWith('.'))
    {
        str.chop(1);
    }

    return str;
}

class ChartView
{
public:
    QRect area;
    qreal maxY{0.0};
    int spacing{0};
    int barWidth{0};
};

static void chartRenderMeyerCurve(QPainter &painter,
                                  const Chart::Curve &curve,
                                  double ha,
                                  const QVector<qreal> &DclassI,
                                  const QVector<qreal> &Dclass,
                                  const QVector<qreal> &Nclass,
                                  const QRect &chartArea,
                                  qreal maxY,
                                  int barWidth,
                                  int spacing)
{
    if (Dclass.size() < 2 || Nclass.size() != Dclass.size())
    {
        return;
    }

    qreal a = curve.a;
    qreal k = curve.k;

    // Step 1: Compute model points
    QVector<QPointF> curvePoints;
    for (int i = 0; i < Dclass.size(); ++i)
    {
        qreal DI = DclassI[i];
        qreal D = Dclass[i];
        qreal N;

        if (curve.id == "Beech")
        {
            N = k * std::exp(-a * (D + 2.5));
        }
        else
        {
            N = k / a * std::exp(-a * D) * (1.0 - std::exp(-a * 5.0));
        }

        N /= ha;

#if defined(FORDIL_DEBUG_CHART)
        qDebug() << "D" << D << "N" << N << "a" << a << "k" << k;
#endif

        qreal x = chartArea.left() + spacing + DI * (barWidth + spacing) + barWidth / 2;
        qreal y = chartArea.bottom() - (N / maxY) * chartArea.height();

        curvePoints.append(QPointF(x, y));
    }

    // Step 2: Draw the curve
    QPen curvePen(curve.color, 2, Qt::SolidLine);
    painter.setPen(curvePen);
    painter.drawPolyline(curvePoints.data(), curvePoints.size());
}

static void chartDrawCurve(QPainter &painter,
                           const QList<QList<qreal>> &stackedData,
                           double ha,
                           const ChartView &view,
                           const Chart::Curve &curve)
{
#if defined(FORDIL_DEBUG_CHART)
    qDebug() << "chartDrawCurve" << curve.id << "enabled" << curve.enabled;
#endif

    if (!curve.enabled)
    {
        return;
    }

    QVector<qreal> DclassI;
    QVector<qreal> Dclass;   // Diameter class midpoints (e.g., 5, 10, 15...)
    QVector<qreal> Nclass;   // Frequency per class (sum of stacked bars)

    for (int i = 0; i < stackedData.size(); ++i)
    {
        qreal D = i * 5 + 2.5;  // assuming 5 cm bins
        qreal N = std::accumulate(stackedData[i].begin(), stackedData[i].end(), 0.0);

        if (D > 0.0)
        {
            DclassI.append(i);
            Dclass.append(D);
            Nclass.append(N);
        }
    }

    chartRenderMeyerCurve(painter, curve, ha, DclassI, Dclass, Nclass,
                          view.area, view.maxY, view.barWidth, view.spacing);
}

static void chartDrawCurves(QPainter &painter,
                            Data &data,
                            const QList<QList<qreal>> &stackedData,
                            double ha,
                            const ChartView &view,
                            const Chart::Options &options)
{
#if defined(FORDIL_DEBUG_CHART)
    qDebug() << "chartDrawCurves";
#endif

    if (options.treeDistributionType != "NHA")
    {
        return;
    }

    for (size_t i = 0; i < options.curves.size(); i++)
    {
        chartDrawCurve(painter, stackedData, ha, view, options.curves[i]);
    }

    // Legend
    int legendX = view.area.right();
    int legendY = view.area.top() + 100;
    int legendSpacing = 5;

    for (const auto &curve : options.curves)
    {
        if (!curve.enabled)
        {
            continue;
        }

        QFontMetrics fm(painter.font());
        int textWidth = fm.horizontalAdvance(curve.name);
        int textHeight = fm.height();

        QPen curvePen(curve.color, 2, Qt::SolidLine);
        painter.setPen(curvePen);
        int x = legendX - textWidth - legendSpacing;
        painter.drawText(x, legendY, data.text(curve.name));

        legendY += textHeight;
        legendY += legendSpacing;
    }
}

static void chartDrawStackedBarChart(QPainter &painter,
                                     Data &data,
                                     const QList<QList<qreal>> &stackedData,
                                     const QStringList &xLabels,
                                     const QStringList &seriesLabels,
                                     const QList<QColor> &colors,
                                     const QString &xAxisTitle,
                                     const QString &yAxisTitle,
                                     double ha,
                                     const Chart::Options &options,
                                     bool xLabelsInside = false)
{
    // Draw Chart
    const int width = painter.device()->width();
    const int height = painter.device()->height();

    // Scaled margins
    const int marginLeft = width * 0.12;
    const int marginRight = width * 0.05;
    const int marginTop = height * 0.12;  // More space for legend
    const int marginBottom = height * 0.12;

    QRect chartArea(marginLeft, marginTop, width - marginLeft - marginRight, height - marginTop - marginBottom);

    // Fonts
    QFont axisFont("Arial", qMax(8, width / 30));
    QFont labelFont("Arial", qMax(9, width / 30));
    QFont labelFont2("Arial", qMax(10, width / 20));
    QFont titleFont("Arial", qMax(10, width / 30), QFont::Bold);

    QFontMetrics axisFontMetrics(axisFont);
    int axisFontHeight = axisFontMetrics.height();

    QFontMetrics labelFontMetrics(labelFont);
    int labelFontHeight = labelFontMetrics.height();

    QFontMetrics labelFont2Metrics(labelFont2);
    int labelFont2Height = labelFont2Metrics.height();

    painter.setFont(axisFont);

    // Determine Y range
    qreal maxY = 0;
    for (const auto &bar : stackedData)
    {
        maxY = std::max(maxY, std::accumulate(bar.begin(), bar.end(), 0.0));
    }

    int yTicks = 6;
    qreal yStep = chartNiceStep(maxY, yTicks - 1);
    qreal maxVal = yStep * std::ceil(maxY / yStep);

#if defined(FORDIL_DEBUG_CHART)
    qDebug() << "maxY" << maxY << "maxVal" << maxVal << "yStep" << yStep;
#endif

    int yLabelOffset = 10;

    // Grid lines and labels
    painter.setPen(Qt::lightGray);
    for (int i = 0; i < yTicks; ++i)
    {
        qreal val = i * yStep;
#if defined(FORDIL_DEBUG_CHART)
        qDebug() << "val" << val << "= i" << i << "* yStep" << yStep;
#endif
        int y = chartArea.bottom() - static_cast<int>((val / maxVal) * chartArea.height());
        painter.drawLine(chartArea.left(), y, chartArea.right(), y);

        int intVal = qRound(val);
        QString strVal = QString::number(intVal);

        painter.setPen(Qt::black);
        painter.drawText(chartArea.left() - yLabelOffset - 30, y + 5, strVal);
        painter.setPen(Qt::lightGray);
    }

    // X Labels and bars
    int numBars = stackedData.size();
    int numSeries = stackedData[0].size();
    int spacing = chartArea.width() / (numBars * 2 + 1);
    int barWidth = spacing;
    int barWidth2 = barWidth / 2;

    if (!xLabelsInside)
    {
        barWidth2 = barWidth;
    }

    for (int i = 0; i < numBars; ++i)
    {
        int x = chartArea.left() + spacing + i * (barWidth + spacing);
        int y = chartArea.bottom();

        for (int j = 0; j < numSeries; ++j)
        {
            qreal val = stackedData[i][j];
            int barHeight = static_cast<int>((val / maxVal) * chartArea.height());
            QRect barRect(x, y - barHeight, barWidth2, barHeight);
            painter.setBrush(colors[j]);
            painter.setPen(Qt::black);
            painter.drawRect(barRect);

            y -= barHeight;
        }

        bool drawTick = false;

        if (xLabelsInside)
        {
            painter.setFont(labelFont2);
            painter.setPen(Qt::black);

            x = chartArea.left() + spacing + i * (barWidth + spacing) + labelFont2Height + 5;
            y = chartArea.bottom() - 10;

            painter.save();
            painter.translate(x, y);
            painter.rotate(-90);
            painter.drawText(0, 0, xLabels.value(i));
            painter.restore();
        }
        else
        {
            QRect textRect(x, chartArea.bottom() + 5 + axisFontHeight, barWidth * 2, axisFontHeight);
            painter.setFont(axisFont);
            painter.setPen(Qt::black);
            bool ok = false;
            int xLabelInt = xLabels.value(i).toInt(&ok);
            if (ok)
            {
                if (xLabelInt % 10 == 0)
                {
                    painter.drawText(textRect.x(), textRect.y(), xLabels.value(i));
                    drawTick = true;
                }
            }
            else
            {
                painter.drawText(textRect.x(), textRect.y(), xLabels.value(i));
            }
        }

        if (drawTick)
        {
            y = chartArea.bottom();
            int xt = x + (barWidth2 / 2);
            //painter.setPen(Qt::lightGray);
            painter.drawLine(xt, y, xt, y + 7);
        }
    }

    //painter.setPen(Qt::black);

    // Axis titles
    painter.setFont(titleFont);
    painter.drawText(width / 2 - 60, height - 10, xAxisTitle);
    painter.save();
    painter.translate(20, height / 2);
    painter.rotate(-90);
    painter.drawText(0, 0, yAxisTitle);
    painter.restore();

    // Draw Legend (top-right corner)
    int legendBoxSize = width / 40;
    int legendSpacing = width / 80;
    int legendX = chartArea.right() - 150;
    int legendY = chartArea.top() - 40;

    painter.setFont(labelFont);
    painter.setPen(Qt::black);

    for (int i = 0; i < seriesLabels.size(); ++i)
    {
        QRect colorRect(legendX, legendY + i * (legendBoxSize + 10), legendBoxSize, legendBoxSize);
#if defined(FORDIL_DEBUG_CHART)
        qDebug() << "legend" << i << "color" << colors[i].name() << "text" << seriesLabels[i];
#endif
        painter.setBrush(colors[i]);
        painter.drawRect(colorRect);

        painter.drawText(colorRect.right() + 5, colorRect.top() + legendBoxSize - 3, seriesLabels[i]);
    }

    ChartView view;
    view.area = chartArea;
    view.maxY = maxVal;
    view.spacing = spacing;
    view.barWidth = barWidth;

    chartDrawCurves(painter, data, stackedData, ha, view, options);
}

void Chart::drawTreeDistribution(QPainter &painter, Data &data, const Options &options)
{
    // Prepare data
    Result::StackedBarChart result;
    Result::treesDistribution(result, data, options.treeDistributionType);

    // Convert data
    QList<QList<qreal>> stackedData;
    QStringList xLabels;
    QStringList seriesLabels;
    QList<QColor> colors;
    QString xAxisTitle;
    QString yAxisTitle;

    for (const auto &it : result.items)
    {
        QList<qreal> values;
        for (const auto &v : it.values)
        {
            values.push_back(v);
        }
        stackedData.push_back(values);
        xLabels.push_back(it.category);
    }

    for (const auto &it : result.sets)
    {
        seriesLabels.push_back(it.label);
        colors.push_back(it.color);
    }

    xAxisTitle = result.xLabel;
    yAxisTitle = result.yLabel;

    // Draw graph
    chartDrawStackedBarChart(painter,
                             data,
                             stackedData,
                             xLabels,
                             seriesLabels,
                             colors,
                             xAxisTitle,
                             yAxisTitle,
                             data.site().ha(),
                             options);
}

static void chartDrawPieChartLabel(QPainter &painter,
                                   const QRectF &pieRect,
                                   int startAngle,
                                   int spanAngle,
                                   const QString &label)
{
    if (label.isEmpty())
    {
        return;
    }

    QRectF pie = pieRect;
    QPointF c = pie.center();
    double rx = pie.width() / 2.0;
    double ry = pie.height() / 2.0;

    // Convert angles to degrees (Qt uses 1/16 deg)
    double startDeg = startAngle / 16.0;
    double spanDeg  = spanAngle  / 16.0;
    double midDeg   = startDeg + spanDeg / 2.0;

    // Point at ~55% radius (works for ellipse too)
    double rFactor = 0.55;
    double rad = qDegreesToRadians(midDeg);
    QPointF pos(
        c.x() + std::cos(rad) * rx * rFactor,
        c.y() - std::sin(rad) * ry * rFactor  // minus because Qt Y axis goes down
    );

    // Text content
    QString labelShort = label.split(' ', Qt::SkipEmptyParts).value(0);
    //QString labelShort = label;

    painter.save();

    // Move to label position, rotate with slice
    painter.translate(pos);
    double textRot = -midDeg; // negative because screen Y is down

    // Optional: keep text upright
    double norm = std::fmod(midDeg, 360.0);
    if (norm < 0) norm += 360.0;
    bool upsideDown = (norm > 90.0 && norm < 270.0);
    if (upsideDown)
        textRot += 180.0;

    painter.rotate(textRot);

    // Draw centered text around (0,0)
    QFontMetrics fm(painter.font());
    QRect textRect = fm.boundingRect(labelShort);
    textRect.adjust(-6, -3, 6, 3); // padding
    textRect.moveCenter(QPoint(0, 0));

    // Optional: small translucent background for readability
    painter.drawText(textRect, Qt::AlignCenter, labelShort);

    painter.restore();
}

static void chartDrawPieChart(QPainter &painter,
                              std::vector<Result::Slice> &slices,
                              bool italics = false)
{
    int imageWidth = painter.device()->width();
    int imageHeight = painter.device()->height();

    // Compute total (does not have to be 100; any sum works)
    int nSlices = static_cast<int>(slices.size());
    double total = 0.0;
    for (const auto &s : slices)
    {
        total += s.value;
    }

    if (total < 1e-6)
    {
        QString empty = QCoreApplication::translate("Backend", "None");
        slices.push_back({empty, 100.0, "#cccccc"});
        // keep the same nSlices
        nSlices++;
        total = 100.0;
    }

    // Layout: legend on top, pie under it
    int fontPointSize = imageHeight / 25;
    QFont font("Arial", fontPointSize);
    if (italics)
    {
        font.setItalic(true);
    }

    QFont boldFont("Arial", fontPointSize);
    boldFont.setBold(true);

    QFontMetrics fm(font);

    painter.setFont(font);

    const int margin = 10;
    int legendBoxSize = 18;
    int legendHeight = fontPointSize;
    int legendSpacing = 6;
    int legendSize = nSlices * (legendHeight + legendSpacing);

    int sideH = imageHeight - legendSize - (margin * 2); // leave space for legend
    int sideW = imageWidth - (margin * 2);
    int side = sideW < sideH ? sideW : sideH;
    QRectF pieRect(
        (imageWidth - side) / 2.0,
        legendSize + margin,
        side,
        side
    );

    // Angles in Qt are in 1/16 degrees
    int startAngle = 90 * 16;
    int totalAngle = 360 * 16;
    int usedAngle = 0;
    for (size_t i = 0; i < slices.size(); i++)
    {
        const auto &s = slices[i];

        int spanAngle;

        if (i == slices.size() - 1)
        {
            // Last slice: take remaining angle
            spanAngle = - (totalAngle - usedAngle);
        }
        else
        {
            double percent = s.value / total;
            spanAngle = static_cast<int>(-percent * 360.0 * 16.0);
            usedAngle += -spanAngle; // accumulate positive amount
        }

        if (s.value > 0.0)
        {
            painter.setPen(Qt::NoPen);
            painter.setBrush(s.color);
            painter.drawPie(pieRect, startAngle, spanAngle);
        }

        startAngle += spanAngle;
    }

    startAngle = 90 * 16;
    for (const auto &s : slices)
    {
        double percent = s.value / total;
        int spanAngle = static_cast<int>(-percent * 360.0 * 16.0);

        if (s.value > 0)
        {
            painter.setPen(Qt::black);
            painter.setBrush(Qt::NoBrush);
            chartDrawPieChartLabel(painter, pieRect, startAngle, spanAngle, s.label);
        }

        if (percent < 0.025)
        {
            break;
        }

        startAngle += spanAngle;
    }

    // Draw legend
    int legendLeft = margin;

    painter.setPen(Qt::black);
    for (int i = 0; i < slices.size(); ++i)
    {
        const auto &s = slices[i];
        int y = margin + i * (legendHeight + legendSpacing);

        // color box
        QRect boxRect(legendLeft,
                      y + ((legendHeight - legendBoxSize) / 2),
                      legendBoxSize,
                      legendBoxSize);
        painter.setBrush(s.color);
        painter.drawRect(boxRect);

        // text label
        QString text1 = s.label;
        painter.setFont(font);
        painter.drawText(
            legendLeft + legendBoxSize + 8,
            y + legendHeight,
            text1
        );

        // text %
        QString text2 = " (" + QString::number(s.value, 'f', 1) + "%)";
        painter.setFont(boldFont);
        int text1Width = fm.horizontalAdvance(text1);
        painter.drawText(
            legendLeft + legendBoxSize + 8 + text1Width,
            y + legendHeight,
            text2
        );
    }
}

static void chartDrawPieChartUpdate(QPainter &painter,
                                    std::vector<Result::Slice> &slices,
                                    bool italics = false)
{
    std::vector<Result::Slice> slicesSorted;

    // Remove 0
    for (size_t i = 0; i < slices.size(); i++)
    {
        if (slices[i].value > 0.0)
        {
            slicesSorted.push_back(slices[i]);
        }
    }

    // Sort
    std::sort(slicesSorted.begin(), slicesSorted.end(),
              [](const Result::Slice& a, const Result::Slice& b) {
                  return a.value > b.value;   // descending
              });

    chartDrawPieChart(painter, slicesSorted, italics);
}

static void chartDrawPieChartType(QPainter &painter,
                                  const QString &type,
                                  const Result::SlicesCut &slicesCut,
                                  bool italics = false)
{
    std::vector<Result::Slice> slices;

    if (type == "Before")
    {
        slices = slicesCut.before;
    }
    else if (type == "Cut")
    {
        slices = slicesCut.cut;
    }
    else if (type == "After")
    {
        slices = slicesCut.after;
    }

    chartDrawPieChartUpdate(painter, slices, italics);
}

void Chart::drawQuality(QPainter &painter, const QString &type, Data &data)
{
    Result::SlicesCut result;
    Result::quality(result, data);
    chartDrawPieChartType(painter, type, result);
}

void Chart::drawQualityEvolution(QPainter &painter, Data &data)
{
    double result = Result::qualityEvolution(data);

    int imageWidth = painter.device()->width();
    int imageHeight = painter.device()->height();

    int margin = 5;

    QRect rect(margin,
               margin,
               imageWidth - (2 * margin),
               imageHeight - (2 * margin));

    QRect imageRect(margin,
                    margin,
                    (imageWidth / 2) - (2 * margin),
                    (imageHeight) - (2 * margin));

    int fontPointSize = imageHeight / 5;
    QFont font("Arial", fontPointSize);
    painter.setFont(font);

    QImage image;
    QString text;
    QTextOption textOption;

    // result < 0.0 : worse
    // result > 0.0 : better
    // else           same

    if (result < 0.0)
    {
        image.load(":/images/ui_status_bad.png");
        text = QCoreApplication::translate("Backend", "Worsened\nquality");
        // textOption.setAlignment(Qt::AlignRight | Qt::AlignBottom);
        textOption.setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
    else if (result > 0.0)
    {
        image.load(":/images/ui_status_good.png");
        text = QCoreApplication::translate("Backend", "Improved\nquality");
        // textOption.setAlignment(Qt::AlignRight | Qt::AlignTop);
        textOption.setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
    else
    {
        image.load(":/images/ui_status_same.png");
        text = QCoreApplication::translate("Backend", "Unchanged\nquality");
        textOption.setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }

    // Image
    painter.drawImage(imageRect, image, image.rect());

    // Text
    painter.setPen(Qt::black);
    painter.setBrush(Qt::NoBrush);
    painter.drawText(rect, text, textOption);

    // painter.drawText(textRect, Qt::AlignCenter, labelShort);
}

void Chart::drawComposition(QPainter &painter, const QString &type, Data &data)
{
    Result::SlicesCut result;
    Result::composition(result, data);
    chartDrawPieChartType(painter, type, result, true);
}

void Chart::drawChoicesAssessment(QPainter &painter, Data &data)
{
    std::vector<Result::Slice> slices;
    Result::choicesAssessment(slices, data);
    chartDrawPieChartUpdate(painter, slices);
}

// Tables
void chartDrawTable(QPainter &painter, const Chart::Table &table)
{
    int imageWidth = painter.device()->width();
    int imageHeight = painter.device()->height();

    // bgr
    // QRect imageRect(0, 0, imageWidth, imageHeight);
    // painter.setPen(Qt::black);
    // painter.setBrush(Qt::white);
    // painter.drawRect(imageRect);

    int rowCount = static_cast<int>(table.rows.size());
    if (rowCount < 1)
    {
        return;
    }
    int colCount = static_cast<int>(table.rows[0].size());
    if (colCount < 1)
    {
        return;
    }

    int margin = 2;
    int spacing = 1;
    int rowWidth = imageWidth - (margin * 2);
    int cellWidth = (rowWidth - ((colCount - 1) * spacing)) / colCount;
    int cellHeight = (imageWidth / 10) - spacing;
    int cellMargin = 2;

    QFont fontValues("Arial", (cellHeight / 3), QFont::Bold);
    painter.setFont(fontValues);

    QFont fontItalic = fontValues;
    fontItalic.setItalic(true);

    for (int r = 0; r < rowCount; r++)
    {
        for (int c = 0; c < colCount; c++)
        {
            const Chart::Cell &cell = table.rows[r][c];

            int x = margin + (c * cellWidth) + (c * spacing);
            int y = margin + (r * cellHeight) + (r * spacing);
            int cw = cellWidth;

            if (cell.rowSpan)
            {
                x = margin;
                cw = rowWidth;
            }

            QRect cellRect(x, y, cw, cellHeight);

            // Background
            if (cell.color.isValid())
            {
                painter.setPen(Qt::NoPen);
                painter.setBrush(cell.color);
                painter.drawRect(cellRect);
            }
            else
            {
                painter.setBrush(Qt::NoBrush);
            }

            // Text alignment
            QTextOption opt;
            if (cell.rowSpan)
            {
                opt.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            }
            else
            {
                //opt.setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                opt.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            }

            // Text font
            if (cell.italic)
            {
                painter.setFont(fontItalic);
            }
            else
            {
                painter.setFont(fontValues);
            }

            // Text color
            if (cell.textColor.isValid())
            {
                painter.setPen(cell.textColor);
            }
            else
            {
                painter.setPen(Qt::black);
            }

            QRect cellTextRect = cellRect.adjusted(cellMargin, cellMargin, -cellMargin, -cellMargin);
            painter.drawText(cellTextRect, cell.text, opt);
        }
    }
}

static QString chartValueImprovement(const QString &beforeStr, const QString &afterStr)
{
    double before = beforeStr.toDouble();
    double after = afterStr.toDouble();

    if (after > before)
    {
        return QCoreApplication::translate("Backend", "Improved");
    }

    if (after < before)
    {
        return QCoreApplication::translate("Backend", "Worsened");
    }

    return QCoreApplication::translate("Backend", "Unchanged");
}

static QString chartValueImprovementColor(const QString &beforeStr, const QString &afterStr)
{
    double before = beforeStr.toDouble();
    double after = afterStr.toDouble();

    if (after > before)
    {
        return "#6cda91";
    }

    if (after < before)
    {
        return "#f86b6f";
    }

    return "#c6b098";
}

void Chart::drawTreeStatistics(QPainter &painter, Data &data, const Options &options)
{
    // Generate statistical results from the input data
    Result::TreeStatistics result;
    Result::treeStatistics(result, data, options.showAllTreeStatisticsSpecies);

    Chart::Table table;

    for (size_t i = 0; i < result.species.size(); i++)
    {
        Result::TreeStatisticsSpecies &species = result.species[i];

        std::vector<Chart::Cell> header(7);

        //header[0].text = species.fullName;
        //header[0].rowSpan = true;

        if (i == 0)
        {
            header[0].text = species.fullName;

            header[1].text = "G/ha";
            header[2].text = "%";
            header[3].text = "V/ha";
            header[4].text = "%";
            header[5].text = "N/ha";
            header[6].text = "%";

            for (size_t j = 1; j < 7; j++)
            {
                header[j].textColor = "#FFFFFF";
                header[j].color = "#26787A";
            }
        }
        else
        {
            header[0].text = species.fullName;
            header[0].rowSpan = true;
            header[0].italic = true;
        }

        table.rows.push_back(header);

        for (size_t j = 0; j < 3; j++)
        {
            std::vector<Chart::Cell> row(7);

            QString label;
            Result::TreeStatisticsComponent *c;
            if (j == 0)
            {
                label = QCoreApplication::translate("Backend", "Before");
                c = &species.before;
            }
            else if (j == 1)
            {
                label = QCoreApplication::translate("Backend", "Fell");
                c = &species.cut;
            }
            else
            {
                label = QCoreApplication::translate("Backend", "After");
                c = &species.after;
            }

            row[0].text = label;

            row[1].text = chartNumber(c->gha);
            row[2].text = chartNumber(c->ghaPercent);

            row[3].text = chartNumber(c->volume);
            row[4].text = chartNumber(c->volumePercent);

            row[5].text = chartNumber(c->n);
            row[6].text = chartNumber(c->nPercent);

            for (size_t k = 0; k < 7; k++)
            {
                if (k == 0)
                {
                    row[k].textColor = "#FFFFFF";
                    if (j == 1)
                    {
                        row[k].color = "#41B28E";
                    }
                    else
                    {
                        row[k].color = "#26787A";
                    }
                }
                else
                {
                    if (j == 1)
                    {
                        row[k].color = "#E7B922";
                    }
                    else
                    {
                        row[k].color = "#B9E5D7";
                    }
                }
            }

            table.rows.push_back(row);
        } 
    }

    chartDrawTable(painter, table);
}

static Chart::Table chartValuesToTable(const Result::ValuesList &result)
{
    Chart::Table table;

    table.rows.resize(4);

    for (size_t r = 0; r < table.rows.size(); r++)
    {
        table.rows[r].resize(1);
    }

    table.rows[1][0].text = QCoreApplication::translate("Backend", "Before");
    table.rows[1][0].textColor = "#FFFFFF";
    table.rows[1][0].color = "#26787A";

    table.rows[2][0].text = QCoreApplication::translate("Backend", "Fell");
    table.rows[2][0].textColor = "#FFFFFF";
    table.rows[2][0].color = "#41B28E";

    table.rows[3][0].text = QCoreApplication::translate("Backend", "After");
    table.rows[3][0].textColor = "#FFFFFF";
    table.rows[3][0].color = "#26787A";

    for (size_t i = 0; i < result.values.size(); i++)
    {
        size_t c = i + 1;
        table.rows[0].resize(c + 1);
        table.rows[1].resize(c + 1);
        table.rows[2].resize(c + 1);
        table.rows[3].resize(c + 1);

        table.rows[0][c].text = result.values[i].label;
        table.rows[0][c].textColor = "#FFFFFF";
        table.rows[0][c].color = "#26787A";

        table.rows[1][c].text = chartNumber(result.values[i].before);
        table.rows[1][c].color = "#B9E5D7";

        table.rows[2][c].text = chartNumber(result.values[i].cut);
        table.rows[2][c].color = "#E7B922";

        table.rows[3][c].text = chartNumber(result.values[i].after);
        table.rows[3][c].color = "#B9E5D7";
    }

    return table;
}

void Chart::drawEconomicUserValues(QPainter &painter, Data &data)
{
    Result::ValuesList result;
    Result::economicValuesUser(result, data);
    chartDrawTable(painter, chartValuesToTable(result));
}

void Chart::drawEconomicExpertValues(QPainter &painter, Data &data)
{
    Result::ValuesList result;
    Result::economicValuesExpert(result, data);
    chartDrawTable(painter, chartValuesToTable(result));
}

void Chart::drawGiniIndex(QPainter &painter, Data &data)
{
    // Calculate data
    Result::TreeDistributionIndex result;
    Result::treeDistributionIndex(result, data);

    // Fill cell values
    QString giniBefore = QString::number(result.giniBefore, 'f', 2);
    QString giniAfter = QString::number(result.giniAfter, 'f', 2);

    // Table
    Chart::Table table;
    std::vector<Chart::Cell> row(2);

    row[0].text = QCoreApplication::translate("Backend", "Before");
    row[0].textColor = "#FFFFFF";
    row[0].color = "#26787A";
    row[1].text = giniBefore;
    row[1].color = "#B9E5D7";
    table.rows.push_back(row);

    row[0].text = QCoreApplication::translate("Backend", "After");
    row[1].text = giniAfter;
    table.rows.push_back(row);

    row[0].text = QCoreApplication::translate("Backend", "Improvement");
    row[1].text = chartValueImprovement(giniBefore, giniAfter);
    row[1].color = chartValueImprovementColor(giniBefore, giniAfter);
    table.rows.push_back(row);

    chartDrawTable(painter, table);
}

void Chart::drawThinningFactor(QPainter &painter, Data &data)
{
    Chart::Table table;
    std::vector<Chart::Cell> row(2);
    row[0].text = QCoreApplication::translate("Backend", "K factor");
    row[0].textColor = "#FFFFFF";
    row[0].color = "#26787A";
    double k = Result::thinningFactor(data);
    if (std::isnan(k))
    {
        row[1].text = QCoreApplication::translate("Backend", "N/A");
    }
    else
    {
        row[1].text = QString::number(k, 'f', 2);
    }
    row[1].color = "#B9E5D7";
    table.rows.push_back(row);
    chartDrawTable(painter, table);
}

void Chart::drawShannonIndex(QPainter &painter, Data &data)
{
    // Calculate data
    Result::TreeDistributionIndex result;
    Result::treeDistributionIndex(result, data);

    // Fill cell values
    QString shannonBefore = QString::number(result.shannonBefore, 'f', 2);
    QString shannonAfter = QString::number(result.shannonAfter, 'f', 2);

    // Table
    Chart::Table table;
    std::vector<Chart::Cell> row(2);
    row[0].text = QCoreApplication::translate("Backend", "Before");
    row[0].textColor = "#FFFFFF";
    row[0].color = "#26787A";
    row[1].text = shannonBefore;
    row[1].color = "#B9E5D7";
    table.rows.push_back(row);
    row[0].text = QCoreApplication::translate("Backend", "After");
    row[1].text = shannonAfter;
    table.rows.push_back(row);
    row[0].text = QCoreApplication::translate("Backend", "Improvement");
    row[1].text = chartValueImprovement(shannonBefore, shannonAfter);
    row[1].color = chartValueImprovementColor(shannonBefore, shannonAfter);
    table.rows.push_back(row);
    chartDrawTable(painter, table);
}

void Chart::drawEcologicalValues(QPainter &painter, Data &data)
{
    Result::ValuesList result;
    Result::ecologicalValues(result, data);
    chartDrawTable(painter, chartValuesToTable(result));
}

void Chart::drawEcologicalValuesDeadBio(QPainter &painter, Data &data)
{
    Result::ValuesList result;
    Result::ecologicalValuesDeadBio(result, data);
    chartDrawTable(painter, chartValuesToTable(result));
}

void Chart::drawEcologicalValuesDmh(QPainter &painter, Data &data)
{
    // Prepare data
    Result::ValuesList values;
    Result::ecologicalValuesDmh(values, data);

    Result::StackedBarChart result;

    result.items.resize(values.values.size());
    for (size_t i = 0; i < values.values.size(); i++)
    {
        result.items[i].category = values.values[i].label;
        result.items[i].values.resize(2);
        result.items[i].values[0] = values.values[i].after;
        result.items[i].values[1] = values.values[i].cut;
    }

    result.sets.resize(2);
    result.sets[0].label = QCoreApplication::translate("Backend", "Uncut");
    result.sets[0].color = "#41B280";
    result.sets[1].label = QCoreApplication::translate("Backend", "Cut");
    result.sets[1].color = "#FF0000";

    // Convert data
    QList<QList<qreal>> stackedData;
    QStringList xLabels;
    QStringList seriesLabels;
    QList<QColor> colors;
    QString xAxisTitle;
    QString yAxisTitle;

    for (const auto &it : result.items)
    {
        QList<qreal> values;
        for (const auto &v : it.values)
        {
            values.push_back(v);
        }
        stackedData.push_back(values);
        xLabels.push_back(it.category);
    }

    for (const auto &it : result.sets)
    {
        seriesLabels.push_back(it.label);
        colors.push_back(it.color);
    }

    xAxisTitle = QCoreApplication::translate("Backend", "DMH");
    yAxisTitle = QCoreApplication::translate("Backend", "N");

    // Draw graph
    const Options options;

    chartDrawStackedBarChart(painter,
                             data,
                             stackedData,
                             xLabels,
                             seriesLabels,
                             colors,
                             xAxisTitle,
                             yAxisTitle,
                             1.0,
                             options,
                             true);
}

static void chartDrawSignalLabel(QPainter &p, const QRect &area, int value)
{
    int w = area.width() / 4;
    QRect rect(area.right() - w,
               area.bottom() - w,
               w,
               w);

    p.setPen(Qt::black);
    p.setBrush(Qt::black);
    p.drawRect(rect);

    QString label = QString::number(value);
    p.setPen(Qt::white);
    p.drawText(rect.adjusted(4, 4, -4, -4),
               Qt::AlignHCenter | Qt::AlignVCenter,
               label);
}

static void chartDrawSignalTriangle(QPainter &p, const QRect &area, int percent)
{
    // Clamp input
    percent = std::clamp(percent, 0, 100);

    int x1A = area.left();
    int x2A = area.right();
    int y1A = area.top();
    int y2A = area.bottom();

    int heightA = y2A - y1A;

    int width = heightA / 2;
    int height = width;
    int m = 0;

    int x1 = x2A - width - m;
    int x2 = x2A - m;
    int y1 = y2A - height - m;
    int y2 = y2A - m;

    // Full triangle
    QPoint bottomRight = QPoint(x2, y2);
    QPoint bottomLeft  = QPoint(x1, y2);
    QPoint topRight    = QPoint(x2, y1);

    QPolygon triangle { bottomRight, bottomLeft, topRight };

    p.setBrush(Qt::black);
    p.setPen(Qt::NoPen);
    p.drawPolygon(triangle);

    if (percent > 0)
    {
        // compute fill height relative to bottom edge
        int fillW = width * percent / 100;

        // horizontal line across triangle at given height
        int x = x1 + fillW;

        // find intersection with diagonal (bottomLeft->topRight)
        double t = double(x2 - x) / width;
        int yDiag = y1 + int(t * height);

        QPoint p1 = QPoint(x1, y2);
        QPoint p2 = QPoint(x, y2);
        QPoint p3 = QPoint(x, yDiag);

        QPolygon fillPoly;
        fillPoly << p1 << p2 << p3;

        //p.setBrush(Qt::green);
        p.setBrush(Qt::darkGreen);
        p.setPen(Qt::NoPen);
        p.drawPolygon(fillPoly);
    }

    // draw outline
    p.setPen(Qt::black);
    p.setBrush(Qt::NoBrush);
    p.drawPolygon(triangle);
}

// Typology
void chartDrawWrappedText(QPainter &p, const QRect &area, const QString &text)
{
    QTextOption opt;
    opt.setWrapMode(QTextOption::WordWrap); // enable word wrap
    opt.setAlignment(Qt::AlignLeft | Qt::AlignTop);

    p.drawText(area, text, opt);
}

static void chartDrawTypologyLocal(QPainter &painter,
                                   const TypologyLocal &data,
                                   double scaleY,
                                   double northAngle,
                                   int x,
                                   int y,
                                   int width,
                                   int height)
{
    // grid size
    int cols = 2;
    int rows = 1;

    if (data.grid.size() == 4)
    {
        rows = 2;
    }
    else if (data.grid.size() == 6)
    {
        rows = 3;
    }
    else
    {
        return;
    }

    // area
    int margin = 10;

    int cellH = ((height - 2 * margin) * 2) / 7;
    int cellW = cellH;
    int bottomSpace = cellH / 2;

    // fonts
    int fontSize = cellH / 6;
    QFont font("Arial", fontSize);
    painter.setFont(font);

    // draw grid
    size_t gridIndex = 0;
    for (int c = 0; c < cols; ++c)
    {
        for (int r = 0; r < rows; ++r)
        {
            const TypologyGrid &grid = data.grid[gridIndex];

            QRect cell(x + margin + (c * cellW),
                       y + margin + (r * cellH),
                       cellW,
                       cellH);

            // draw border
            painter.setPen(Qt::black);
            painter.setBrush(grid.color);
            painter.drawRect(cell);

            // draw label in top-left
            QRect shifted = cell.adjusted(1, 0, 0, 0);
            chartDrawWrappedText(painter, shifted, grid.label);

            // draw GHA
            int percent = 0;
            if (grid.Typo_gha_loc == 0)      percent = 14;
            else if (grid.Typo_gha_loc == 1) percent = 29;
            else if (grid.Typo_gha_loc == 2) percent = 43;
            else if (grid.Typo_gha_loc == 3) percent = 57;
            else if (grid.Typo_gha_loc == 4) percent = 71;
            else if (grid.Typo_gha_loc == 5) percent = 85;
            else                             percent = 100;

            chartDrawSignalLabel(painter, cell, grid.Typo_gha_loc);

            gridIndex++;
        }
    }

    // values
    QRect row(x + margin,
              y + height - bottomSpace,
              width - 2 * margin,
              bottomSpace / 2);

    cellW = row.width() / 3;
    cellH = row.height();
    
    QFont fontValues("Arial", (cellH / 5) * 3);
    painter.setFont(fontValues);

    for (int c = 0; c < 3; ++c)
    {
        QRect cell(row.left() + c*cellW, row.top(), cellW, cellH);

        painter.setPen(Qt::black);
        painter.setBrush(Qt::white);
        painter.drawRect(cell);

        QString label;
        QString desc;
        double value;
        if (c == 0)
        {
            value = data.DBH_lw;
            desc = QCoreApplication::translate("Backend", "%lw");
        }
        else if (c == 1)
        {
            value = data.DBH_mw;
            desc = QCoreApplication::translate("Backend", "%mw");
        }
        else
        {
            value = data.DBH_bwvbw;
            desc = QCoreApplication::translate("Backend", "%bwvbw");
        }
        label = QString::number(value, 'f', 1);

        painter.setPen(Qt::black);
        painter.setBrush(Qt::NoBrush);
        painter.drawText(cell, Qt::AlignCenter, label);

        QRect cellD(row.left() + c*cellW, row.top() + cellH, cellW, cellH);
        painter.drawText(cellD, Qt::AlignCenter, desc);
    }
}

void Chart::drawTypologyLocal(QPainter &painter,
                              const Data &data,
                              const Options &options)
{
    double scaleY = data.typology().Scale_y();
    double northAngle = data.typology().North_angle();

    int width = painter.device()->width();
    int height = painter.device()->height();

    chartDrawTypologyLocal(painter,
                           data.typology().localInit(),
                           scaleY,
                           northAngle,
                           0, 0, width / 2, height);

    TypologyLocal after = data.typology().localUser();
    if (options.typologyLocalType != "User")
    {
        after = data.typology().localExpert();
    }

    chartDrawTypologyLocal(painter,
                           after,
                           scaleY,
                           northAngle,
                           width / 2, 0, width / 2, height);

}

void Chart::drawTypologyLocal(QPainter &painter,
                              const Data &data,
                              const QString &when,
                              const Options &options)
{
    double scaleY = data.typology().Scale_y();
    double northAngle = data.typology().North_angle();

    int width = painter.device()->width();
    int height = painter.device()->height();

    if (when == "Before")
    {
        chartDrawTypologyLocal(painter,
                               data.typology().localInit(),
                               scaleY,
                               northAngle,
                               0, 0, width, height);
    }
    else
    {
        QString type = options.typologyLocalType;

        if (type == "User")
        {
            chartDrawTypologyLocal(painter,
                                   data.typology().localUser(),
                                   scaleY,
                                   northAngle,
                                   0, 0, width, height);
        }
        else
        {
            chartDrawTypologyLocal(painter,
                                   data.typology().localExpert(),
                                   scaleY,
                                   northAngle,
                                   0, 0, width, height);
        }
    }
}

static void chartDrawArrow(QPainter &painter,
                           QPointF start,
                           QPointF end,
                           qreal arrowSize = 10.0,
                           qreal stop = 0.0)
{
    QLineF line(start, end);

    if (stop > 0.0)
    {
        qreal len = line.length();
        if (len > stop * 2.0)
        {
            start = line.pointAt(stop / len);
            end   = line.pointAt((len - stop) / len);
            line.setPoints(start, end);
        }
        else
        {
            return;
        }
    }

    painter.drawLine(line);

    if (arrowSize < 1.0)
    {
        return;
    }

    double angle = std::atan2(line.dy(), line.dx());

    QPointF arrowP1 = end - QPointF(std::cos(angle) * arrowSize +
                                    std::sin(angle) * (arrowSize / 2),
                                    std::sin(angle) * arrowSize -
                                    std::cos(angle) * (arrowSize / 2));

    QPointF arrowP2 = end - QPointF(std::cos(angle) * arrowSize -
                                    std::sin(angle) * (arrowSize / 2),
                                    std::sin(angle) * arrowSize +
                                    std::cos(angle) * (arrowSize / 2));

    QPolygonF arrowHead;
    arrowHead << end << arrowP1 << arrowP2;
    painter.drawPolygon(arrowHead);
}

static void chartDrawArrowDown(QPainter &painter, int x, int y, int size)
{
    QPointF a(x - size / 2, y - size);
    QPointF b(x, y);
    QPointF c(x + size / 2, y - size);

    QPolygonF arrowHead;
    arrowHead << a << b << c;
    painter.drawPolygon(arrowHead);
}

static void chartDrawText(QPainter &painter,
                          const QFont &font,
                          const QFontMetrics &fm,
                          int x,
                          int y,
                          const QString &text,
                          const QString &align = "")
{
    int pointSize = font.pointSize();
    int pointSizeHalf = pointSize / 2;
    int textWidth = fm.horizontalAdvance(text);
    int textWidthHalf = textWidth / 2;

    if (align.contains("V"))
    {
        x = x - textWidthHalf;
    }
    else if (align.contains("R"))
    {
        x = x - textWidth;
    }

    if (align.contains("H"))
    {
        y = y + pointSizeHalf;
    }
    else if (align.contains("T"))
    {
        y = y + pointSize;
    }

    painter.drawText(x, y, text);
}

void Chart::drawTypologyGlobal(QPainter &painter, const Data &data, const Options &options)
{
    QString age = options.typologyGlobalAge;

    // zones
    std::vector<QPointF> points(15);
    points[0] = QPointF(0.0, 0.0);

    points[1] = QPointF(0.0, 0.5);
    points[2] = QPointF(0.25, 0.5);
    points[3] = QPointF(0.5, 0.5);

    points[4] = QPointF(0.0, 0.8);
    points[5] = QPointF(0.25, 0.8);
    points[6] = QPointF(0.3, 0.8);
    points[7] = QPointF(0.5, 0.8);
    points[8] = QPointF(0.55, 0.8);
    points[9] = QPointF(0.8, 0.8);

    points[10] = QPointF(0.5, 0.95);
    points[11] = QPointF(0.95, 0.95);

    points[12] = QPointF(0.0, 1.0);
    points[13] = QPointF(0.5, 1.0);
    points[14] = QPointF(1.0, 1.0);

    class ChartTypologyGlobalPoly
    {
    public:
        std::vector<size_t> idx;
        QColor color;
        QString label;

        void set(const std::vector<size_t> &idx_,
                 const QColor &color_,
                 const QString &label_)
        {
            idx = idx_;
            color = color_;
            label = label_;
        }
    };

    std::vector<ChartTypologyGlobalPoly> polygons(8);
    polygons[0].set({0, 1, 3}, "#0088cb", "8, 9");
    polygons[1].set({1, 4, 5, 2}, "#bad8ee", "7");
    polygons[2].set({2, 5, 8}, "#f6d3e5", "6");
    polygons[3].set({2, 8, 9, 3}, "#8157a2", "5");
    polygons[4].set({4, 12, 13, 6}, "#629659", "4");
    polygons[5].set({6, 13, 7}, "#c7dfc7", "3");
    polygons[6].set({7, 10, 11, 9}, "#f5bd99", "2");
    polygons[7].set({10, 13, 14, 11}, "#fac421", "1");

    // draw
    int imageWidth = painter.device()->width();
    int imageHeight = painter.device()->height();

    int spacing = 5;

    // fonts
    int fontPointSize = imageHeight / 25;
    int fontPointSizeHalf = fontPointSize / 2;
    QFont font("Arial", fontPointSize);
    QFontMetrics fm(font);
    painter.setFont(font);

    // draw
    int marginLeft = fm.horizontalAdvance("100%") + fm.height() + spacing;
    int marginRight = fm.horizontalAdvance("100%") / 2;
    int marginTop = fm.height() / 2;
    int marginBottom = fm.height() * 2 + spacing;
    
    int plotWidth = imageWidth - marginLeft - marginRight;
    int plotHeight = plotWidth;

    int plotLeft = marginLeft;
    int plotRight = marginLeft + plotWidth;
    int plotTop = marginTop;
    int plotBottom = marginTop + plotHeight;

    qreal tx = marginLeft;
    qreal ty = marginTop;
    qreal tw = plotWidth;
    qreal th = plotHeight;

    for (size_t i = 0; i < points.size(); i++)
    {
        points[i].setX(points[i].x() * tw + tx);
        points[i].setY(points[i].y() * th + ty);
    }

    painter.setPen(Qt::black);

    for (size_t i = 0; i < polygons.size(); i++)
    {
        auto &poly = polygons[i];
        if (poly.idx.size() < 3)
        {
            continue;
        }

        qreal avgX = 0;
        qreal avgY = 0;

        QPolygonF qpoly;
        for (size_t j = 0; j < poly.idx.size(); j++)
        {
            QPointF &pt = points[poly.idx[j]];
            qpoly << pt;

            avgX += pt.x();
            avgY += pt.y();
        }

        int x = static_cast<int>(avgX / static_cast<qreal>(poly.idx.size()));
        int y = static_cast<int>(avgY / static_cast<qreal>(poly.idx.size()));

        painter.setBrush(poly.color);
        painter.drawPolygon(qpoly);
        painter.drawText(x, y + fontPointSizeHalf, poly.label);
    }

    // axis
    int tickLen = 5;

    // y axis
    QString yAxisLabel = QCoreApplication::translate("Backend",
                         "% Large and very large woods");
    int yAxisLabelLength = fm.horizontalAdvance(yAxisLabel);
    int yAxisLabelStart = plotBottom;
    if (yAxisLabelLength < plotHeight)
    {
        yAxisLabelStart = plotBottom - (plotHeight - yAxisLabelLength) / 2;
    }
    painter.save();
    painter.translate(fm.height(), yAxisLabelStart);
    painter.rotate(-90);
    painter.drawText(0, 0, yAxisLabel);
    painter.restore();

    int yAxisValueX = plotLeft - tickLen;
    int yAxisValueY25 = plotHeight / 4;
    int yAxisValueY50 = plotHeight / 2;
    int yAxisValueY75 = (plotHeight / 2) + (plotHeight / 4);
    chartDrawText(painter, font, fm, yAxisValueX, plotBottom - plotHeight, "100%", "RH");
    chartDrawText(painter, font, fm, yAxisValueX, plotBottom - yAxisValueY75, "75%", "RH");
    chartDrawText(painter, font, fm, yAxisValueX, plotBottom - yAxisValueY50, "50%", "RH");
    chartDrawText(painter, font, fm, yAxisValueX, plotBottom - yAxisValueY25, "25%", "RH");

    // x axis
    int xAxisValueY = plotBottom + tickLen + fontPointSize;
    int xAxisValueX25 = plotWidth / 4;
    int xAxisValueX50 = plotWidth / 2;
    int xAxisValueX75 = (plotWidth / 2) + (plotWidth / 4);
    chartDrawText(painter, font, fm, plotLeft, xAxisValueY, "0%", "V");
    chartDrawText(painter, font, fm, plotLeft + xAxisValueX25, xAxisValueY, "25%", "V");
    chartDrawText(painter, font, fm, plotLeft + xAxisValueX50, xAxisValueY, "50%", "V");
    chartDrawText(painter, font, fm, plotLeft + xAxisValueX75, xAxisValueY, "75%", "V");
    chartDrawText(painter, font, fm, plotLeft + plotWidth, xAxisValueY, "100%", "V");

    int xAxisLabelY = xAxisValueY + spacing + fontPointSize;
    QString xAxisLabel = QCoreApplication::translate("Backend", "% Thin woods");
    chartDrawText(painter, font, fm, plotLeft + xAxisValueX50, xAxisLabelY, xAxisLabel, "V");

    // data points
    class ChartTypologyGlobalDataPoint
    {
    public:
        QString name;
        double sx;
        double sy;
        double x;
        double y;
        QColor color;
        QString label;
    };

    const Typology &typology = data.typology();
    const TypologyGlobal &globalNature = typology.globalNature();
    const TypologyGlobal &globalExpert = typology.globalExpert();
    const TypologyGlobal &globalUser = typology.globalUser();

    std::vector<ChartTypologyGlobalDataPoint> dataPoints;
    if (age == "10 years")
    {
        dataPoints.resize(4);

        dataPoints[0].name = "Initial";
        dataPoints[0].sx = globalNature.Struc_n0_X;
        dataPoints[0].sy = globalNature.Struc_n0_Y;
        dataPoints[0].color = QColor("#FE3531");
        dataPoints[0].label = QCoreApplication::translate("Backend", "Initial structure");

        dataPoints[1].name = "Expert";
        dataPoints[1].sx = globalExpert.Struc_n10_X;
        dataPoints[1].sy = globalExpert.Struc_n10_Y;
        dataPoints[1].color = QColor("#40CF65");
        dataPoints[1].label = QCoreApplication::translate("Backend", "Expert results");

        dataPoints[2].name = "Natural";
        dataPoints[2].sx = globalNature.Struc_n10_X;
        dataPoints[2].sy = globalNature.Struc_n10_Y;
        dataPoints[2].color = QColor("#ED7661");
        dataPoints[2].label = QCoreApplication::translate("Backend", "Natural evolution");

        dataPoints[3].name = "User";
        dataPoints[3].sx = globalUser.Struc_n10_X;
        dataPoints[3].sy = globalUser.Struc_n10_Y;
        dataPoints[3].color = QColor("#3595FC");
        dataPoints[3].label = QCoreApplication::translate("Backend", "User results");
    }
    else
    {
        dataPoints.resize(3);

        dataPoints[0].name = "Initial";
        dataPoints[0].sx = globalNature.Struc_n0_X;
        dataPoints[0].sy = globalNature.Struc_n0_Y;
        dataPoints[0].color = QColor("#FE3531");
        dataPoints[0].label = QCoreApplication::translate("Backend", "Initial structure");

        dataPoints[1].name = "Expert";
        dataPoints[1].sx = globalExpert.Struc_n0_X;
        dataPoints[1].sy = globalExpert.Struc_n0_Y;
        dataPoints[1].color = QColor("#40CF65");
        dataPoints[1].label = QCoreApplication::translate("Backend", "Expert results");

        dataPoints[2].name = "User";
        dataPoints[2].sx = globalUser.Struc_n0_X;
        dataPoints[2].sy = globalUser.Struc_n0_Y;
        dataPoints[2].color = QColor("#3595FC");
        dataPoints[2].label = QCoreApplication::translate("Backend", "User results");
    }


    int pointRadius = 7;

    int legendX = plotLeft + plotWidth / 2 + 20;
    int legendY = plotTop + fontPointSize;
    int legendYStep = fontPointSize + (fontPointSize / 4);

    for (auto &pt : dataPoints)
    {
        pt.x = static_cast<double>(plotLeft) +
               static_cast<double>(plotWidth) * pt.sx * 0.01;

        pt.y = static_cast<double>(plotBottom) -
               static_cast<double>(plotHeight) * pt.sy * 0.01;
    }

    for (size_t i = 0; i < dataPoints.size(); i++)
    {
        auto &pt = dataPoints[i];

        // draw point
        int x = static_cast<int>(std::round(pt.x));
        int y = static_cast<int>(std::round(pt.y));

        painter.setPen(Qt::black);
        painter.setBrush(pt.color);
        painter.drawEllipse(QPoint(x, y), pointRadius, pointRadius);

        // draw legend
        int xL = legendX;
        int yL = legendY + legendYStep * i;

        painter.drawEllipse(QPoint(xL - pointRadius - 3, yL - fontPointSizeHalf),
                            pointRadius,
                            pointRadius);

        painter.setPen(pt.color);
        painter.setBrush(Qt::NoBrush);
        QString label = pt.label;
#if defined(FORDIL_DEBUG_CHART)
        label += " [" + QString::number(pt.sx, 'f', 1) +
                 ", " + QString::number(pt.sy, 'f', 1) + "]";
#endif
        painter.drawText(xL, yL, label);
    }

    // draw lines
    std::map<QString, QPointF> ptMap;
    for (const auto &pt : dataPoints)
    {
        ptMap[pt.name] = QPointF(pt.x, pt.y);
    }

    painter.setPen(Qt::black);
    painter.setBrush(Qt::black);

    chartDrawArrow(painter, ptMap["Initial"], ptMap["User"], 10.0, pointRadius);

    QPen penDotLine(Qt::black);
    penDotLine.setStyle(Qt::DotLine);
    painter.setPen(penDotLine);

    chartDrawArrow(painter, ptMap["Initial"], ptMap["Expert"], 0.0, pointRadius);

    if (ptMap.count("Natural") > 0)
    {
        chartDrawArrow(painter, ptMap["Initial"], ptMap["Natural"], 0.0, pointRadius);
    }

    // test rect
    // QRect plotRect(plotLeft, plotTop, plotWidth, plotHeight);
    // painter.setBrush(Qt::NoBrush);
    // painter.setPen(Qt::black);
    // painter.drawRect(plotRect);
}

static void chartDrawOverallAssessment(QPainter &painter,
                                       const QRect &labelRect,
                                       const QString &label,
                                       const QString &debugText,
                                       const QRect &imageRect,
                                       const std::vector<std::pair<double,QColor>> &gradient,
                                       double valueUser,
                                       double valueExpert,
                                       double max,
                                       bool showMax)
{
    // label
    QFont fontLabel("Arial", labelRect.height() / 5);
    painter.setFont(fontLabel);
    painter.setPen("#41B28E");
    painter.setBrush(Qt::NoBrush);

    // debug label
    if (!debugText.isEmpty())
    {
        painter.drawText(labelRect, Qt::AlignHCenter | Qt::AlignTop, label);

        QFont fontDebugText("Arial", labelRect.height() / 7);
        painter.setFont(fontDebugText);
        painter.drawText(labelRect, Qt::AlignHCenter | Qt::AlignBottom, debugText);
    }
    else
    {
        painter.drawText(labelRect, Qt::AlignCenter | Qt::TextWordWrap, label);
    }

    // bar
    QColor colorNature("#ED7661");
    QColor colorUser("#3595FC");
    QColor colorExpert("#40CF65");

    int marginW = 15;
    int marginH = 5;

    int w = imageRect.width() - marginW * 2;
    int h = imageRect.height() - marginH * 2;
    int ha = 2 * (h / 3);
    int hb = 1 * (h / 3);

    painter.setPen(Qt::black);
    painter.setBrush(Qt::NoBrush);

    // Marks
    QImage imageExpert(":/images/ui_bar_mark_expert.png");
    QImage imageNature(":/images/ui_bar_mark_nature.png");
    QImage imageUser(":/images/ui_bar_mark_user.png");
    QImage imageMax(":/images/ui_attention_sign.png");

    class MarkInfo
    {
    public:
        double value;
        QImage &image;
        double scale;
    };

    std::vector<MarkInfo> markList;

    markList.push_back({0.0, imageNature, 1.0});
    markList.push_back({valueUser, imageUser, 1.0});
    markList.push_back({valueExpert, imageExpert, 1.0});

    if (showMax)
    {
        markList.push_back({max, imageMax, 0.5});
    }

    for (auto &it : markList)
    {
        // Normalize value
        double value = it.value;

        if (value < 0.0)
        {
            value = 0.0;
        }

        if (value > max)
        {
            value = max;
        }

        value = value / max;

        // Position
        int p = static_cast<int>(std::round(value * static_cast<double>(w)));

        QImage &img = it.image;
        // m is target height
        if (img.height() < 1)
        {
            continue;
        }
        int imgH = static_cast<int>(static_cast<double>(ha) * it.scale);
        int imgW = img.width() * imgH / img.height();
        int x = marginW + (p - imgW / 2);
        int y = marginH + ha - imgH;

        if (x + imgW > imageRect.width())
        {
            x = imageRect.width() - imgW;
        }

        QRect dst(imageRect.x() + x, imageRect.y() + y, imgW, imgH);
        painter.drawImage(dst, img, img.rect());
    }

    // Rectangle bar
    QRect bar(imageRect.x() + marginW, imageRect.y() + marginH + ha, w, hb);

    // Create a horizontal gradient
    QLinearGradient grad(bar.topLeft(), bar.topRight());
    for (auto &it : gradient)
    {
        grad.setColorAt(it.first, it.second);
    }

    painter.setBrush(grad);
    painter.drawRect(bar);
}

static void chartDrawOverallAssessmentStructure(QPainter &painter,
                                                const QRect &labelRect,
                                                const QString &label,
                                                const QRect &imageRect,
                                                const Data &data)
{
    const Typology &typology = data.typology();
    const TypologyGlobal &globalNature = typology.globalNature();
    const TypologyGlobal &globalUser = typology.globalUser();
    const TypologyGlobal &globalExpert = typology.globalExpert();

    // Scale representing global typological structure results
    double Xn = globalNature.Struc_n0_X;
    double Yn = globalNature.Struc_n0_Y;

    double Xu = globalUser.Struc_n0_X;
    double Yu = globalUser.Struc_n0_Y;

    double Xe = globalExpert.Struc_n0_X;
    double Ye = globalExpert.Struc_n0_Y;

    double d1 = (Xu - Xn) * (Xu - Xn) + (Yu - Yn) * (Yu - Yn);
    if (d1 > 0.0)
    {
        d1 = std::sqrt(d1);
    }

    double d2 = (Xe - Xu) * (Xe - Xu) + (Ye - Yu) * (Ye - Yu);
    if (d2 > 0.0)
    {
        d2 = std::sqrt(d2);
    }

    // 0 .. structure .. 1
    double structure = d1 + d2;
    if (structure > 0.0)
    {
        structure = d1 / structure;
    }
    if (structure > 1.0)
    {
        structure = 1.0;
    }

    // debug
    QString debugText;
    // QString debugText =  "Xn " + QString::number(Xn, 'f', 1) +
    //                     " Xu " + QString::number(Xu, 'f', 1) +
    //                     " Xe " + QString::number(Xe, 'f', 1) +
    //                     "\nYn " + QString::number(Yn, 'f', 1) +
    //                     " Yu " + QString::number(Yu, 'f', 1) +
    //                     " Ye " + QString::number(Ye, 'f', 1);

    // draw
    std::vector<std::pair<double,QColor>> gradient;
    gradient.push_back({0.0, QColor("#FF0000")});
    gradient.push_back({1.0, QColor("#00B050")});
    chartDrawOverallAssessment(painter, labelRect, label, debugText, imageRect, gradient, structure, 1.0, 1.0, false);
}

static void chartDrawOverallAssessmentBasalArea(QPainter &painter,
                                                const QRect &labelRect,
                                                const QString &label,
                                                const QRect &imageRect,
                                                const Data &data)
{
    const Typology &typology = data.typology();
    const TypologyGlobal &globalNature = typology.globalNature();
    const TypologyGlobal &globalUser = typology.globalUser();
    const TypologyGlobal &globalExpert = typology.globalExpert();

    // Scale representing global typological basal area results
    double Gn = globalNature.Gha_n0;
    double Gu = globalUser.Gha_n0;
    double Ge = globalExpert.Gha_n0;

    double max = 0.5;
    double user = 0.0;
    double expert = 0.0;

    if (Gn > 0.0)
    {
        user = (Gn - Gu) / Gn;
        expert = (Gn - Ge) / Gn;
    }

    // debug
    QString debugText;
    // QString debugText = "Gn=" + QString::number(Gn, 'f', 1) +
    //                    " Gu=" + QString::number(Gu, 'f', 1) +
    //                    " Ge=" + QString::number(Ge, 'f', 1);

    // draw
    std::vector<std::pair<double,QColor>> gradient;
    gradient.push_back({0.0, QColor("#4da52e")});
    gradient.push_back({0.5, QColor("#4da52e")});
    gradient.push_back({1.0, QColor("#FF0000")});
    chartDrawOverallAssessment(painter, labelRect, label, debugText, imageRect, gradient, user, expert, max, true);
}

void Chart::drawOverallAssessment(QPainter &painter, const Data &data)
{
    int w = painter.device()->width();
    int h = painter.device()->height();

    int cw1 = (w / 3) * 1;
    int cw2 = (w / 3) * 2;
    int rh = w / 5;

    QRect strucLabelRect(20, 0, cw1 - 10, rh);
    QRect strucRect(cw1, 0, cw2, rh);
    QRect areaLabelRect(20, rh, cw1 - 10, rh);
    QRect areaRect(cw1, rh, cw2, rh);

    QString strucLabel = QCoreApplication::translate("Backend", "Structure");
    QString areaLabel = QCoreApplication::translate("Backend", "Basal area");

    chartDrawOverallAssessmentStructure(painter, strucLabelRect, strucLabel, strucRect, data);
    chartDrawOverallAssessmentBasalArea(painter, areaLabelRect, areaLabel, areaRect, data);
}

// Valuation
#if 0
static QString chartTreeSelectionReasonToImageSource(TreeSelectionReasonValue reason)
{
    static const std::map<TreeSelectionReasonValue, QString> treeSelectionReasonMap = {
        {TreeSelectionReasonValue::Competition, "qrc:/images/C_1.png"},
        {TreeSelectionReasonValue::Maturity, "qrc:/images/C_2.png"},
        {TreeSelectionReasonValue::PromoteRegeneration, "qrc:/images/C_3.png"},
        {TreeSelectionReasonValue::PromoteRareSpecies, "qrc:/images/C_4.png"},
        {TreeSelectionReasonValue::Sanitary, "qrc:/images/C_5.png"},
        {TreeSelectionReasonValue::Technical, "qrc:/images/C_6.png"},
        {TreeSelectionReasonValue::BadQuality, "qrc:/images/C_7.png"},
        {TreeSelectionReasonValue::Siblings, ""},
        {TreeSelectionReasonValue::Biodiversity, ""},
        {TreeSelectionReasonValue::HabitatTree, ""},
        {TreeSelectionReasonValue::DeadTree, ""}
    };

    auto it = treeSelectionReasonMap.find(reason);
    if (it != treeSelectionReasonMap.end())
    {
        return it->second;
    }

    return "";
}
#endif

QVariantList Chart::hotspotsModelData(const Data &data)
{
    QVariantList list;

    Result::Valuation hotspots;
    Result::hotspots(hotspots, data);

    for (const auto &it : hotspots.reasons)
    {
        QString reason = treeSelectionReasonToString(it.treeSelectionReason);
        QString reasonTr = treeSelectionReasonToStringTr(it.treeSelectionReason);
        // QString reasonImage = chartTreeSelectionReasonToImageSource(it.treeSelectionReason);

        list << QVariantMap{{"reason", reason},
                            {"reasonTr", reasonTr},
                            {"score", it.text},
                            {"color", it.color},
                            {"reasonImageSource", ""}, // reasonImage},
                            {"detailImageSource", "qrc:/images/ui_plus.png"}};
    }

    return list;
}

void Chart::requestImage(QPainter &painter,
                         const QString &imageId,
                         Data &data,
                         const Chart::Options &options)
{
    if (imageId == "mapImage")
    {
        MapPainter::draw(painter, data);
    }
    else if (imageId == "treeStatisticsImage")
    {
        Chart::drawTreeStatistics(painter, data, options);
    }
    else if (imageId == "economicUserValuesImage")
    {
        Chart::drawEconomicUserValues(painter, data);
    }
    else if (imageId == "economicExpertValuesImage")
    {
        Chart::drawEconomicExpertValues(painter, data);
    }
    else if (imageId == "treeDistributionImage")
    {
        Chart::drawTreeDistribution(painter, data, options);
    }
    else if (imageId == "giniIndexImage")
    {
        Chart::drawGiniIndex(painter, data);
    }
    else if (imageId == "thinningFactorImage")
    {
        Chart::drawThinningFactor(painter, data);
    }
    else if (imageId == "qualityBeforeImage")
    {
        Chart::drawQuality(painter, "Before", data);
    }
    else if (imageId == "qualityCutImage")
    {
        Chart::drawQuality(painter, "Cut", data);
    }
    else if (imageId == "qualityAfterImage")
    {
        Chart::drawQuality(painter, "After", data);
    }
    else if (imageId == "qualityEvolutionImage")
    {
        Chart::drawQualityEvolution(painter, data);
    }
    else if (imageId == "compositionBeforeImage")
    {
        Chart::drawComposition(painter, "Before", data);
    }
    else if (imageId == "compositionCutImage")
    {
        Chart::drawComposition(painter, "Cut", data);
    }
    else if (imageId == "compositionAfterImage")
    {
        Chart::drawComposition(painter, "After", data);
    }
    else if (imageId == "shannonIndexImage")
    {
        Chart::drawShannonIndex(painter, data);
    }
    else if (imageId == "choicesAssessmentImage")
    {
        Chart::drawChoicesAssessment(painter, data);
    }
    else if (imageId == "ecologicalValuesImage")
    {
        Chart::drawEcologicalValues(painter, data);
    }
    else if (imageId == "ecologicalValuesDeadBioImage")
    {
        Chart::drawEcologicalValuesDeadBio(painter, data);
    }
    else if (imageId == "ecologicalValuesDmhImage")
    {
        Chart::drawEcologicalValuesDmh(painter, data);
    }
    else if (imageId == "typologyLocalImage")
    {
        Chart::drawTypologyLocal(painter, data, options);
    }
    else if (imageId == "typologyLocalBeforeImage")
    {
        Chart::drawTypologyLocal(painter, data, "Before", options);
    }
    else if (imageId == "typologyLocalAfterImage")
    {
        Chart::drawTypologyLocal(painter, data, "After", options);
    }
    else if (imageId == "typologyGlobalImage")
    {
        Chart::drawTypologyGlobal(painter, data, options);
    }
    else if (imageId == "overallAssessmentImage")
    {
        Chart::drawOverallAssessment(painter, data);
    }
}

double Chart::imageHeight(double width,
                          const QString &imageId,
                          Data &data,
                          const Chart::Options &options)
{
    double height{0.0};

    if (imageId == "mapImage")
    {
        height = width;
    }
    else if (imageId == "treeStatisticsImage")
    {
        Result::TreeStatistics result;
        Result::treeStatistics(result, data, options.showAllTreeStatisticsSpecies);

        double n = static_cast<double>(result.species.size() * 4);
        height = width * 0.1 * n;
    }
    else if (imageId == "economicUserValuesImage")
    {
        height = width * 0.1 * 4;
    }
    else if (imageId == "economicExpertValuesImage")
    {
        height = width * 0.1 * 4;
    }
    else if (imageId == "treeDistributionImage")
    {
        height = width;
    }
    else if (imageId == "giniIndexImage")
    {
        height = width * 0.1 * 3;
    }
    else if (imageId == "thinningFactorImage")
    {
        height = width * 0.1 * 1;
    }
    else if (imageId == "qualityBeforeImage")
    {
        height = width;
    }
    else if (imageId == "qualityCutImage")
    {
        height = width;
    }
    else if (imageId == "qualityAfterImage")
    {
        height = width;
    }
    else if (imageId == "qualityEvolutionImage")
    {
        height = width / 2; // small width 250
    }
    else if (imageId == "compositionBeforeImage")
    {
        height = width;
    }
    else if (imageId == "compositionCutImage")
    {
        height = width;
    }
    else if (imageId == "compositionAfterImage")
    {
        height = width;
    }
    else if (imageId == "shannonIndexImage")
    {
        height = width * 0.1 * 3;
    }
    else if (imageId == "choicesAssessmentImage")
    {
        height = width;
    }
    else if (imageId == "ecologicalValuesImage")
    {
        height = width * 0.1 * 4;
    }
    else if (imageId == "ecologicalValuesDeadBioImage")
    {
        height = width * 0.1 * 4;
    }
    else if (imageId == "ecologicalValuesDmhImage")
    {
        height = width;
    }
    else if (imageId == "typologyLocalImage")
    {
        height = width * 0.85;
    }
    else if (imageId == "typologyLocalBeforeImage")
    {
        height = width;
    }
    else if (imageId == "typologyLocalAfterImage")
    {
        height = width;
    }
    else if (imageId == "typologyGlobalImage")
    {
        height = width;
    }
    else if (imageId == "overallAssessmentImage")
    {
        height = width * 0.5;
    }

    return height;
}
