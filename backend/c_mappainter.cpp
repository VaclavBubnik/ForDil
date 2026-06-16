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

#include "c_mappainter.h"
#include <QDebug>

// Utility to find best fitting font size
static int findOptimalFontSize(QPainter &painter, const QString &text, const QRect &rect, int minSize = 1, int maxSize = 100)
{
    int optimalSize = minSize;

    while (minSize <= maxSize)
    {
        int mid = (minSize + maxSize) / 2;
        QFont font = painter.font();
        font.setPointSize(mid);
        painter.setFont(font);
        QFontMetrics fm(font);

        QRect textRect = fm.boundingRect(rect, Qt::AlignCenter, text);
        if (textRect.width() <= rect.width() && textRect.height() <= rect.height())
        {
            optimalSize = mid;
            minSize = mid + 1;
        }
        else
        {
            maxSize = mid - 1;
        }
    }

    return optimalSize;
}

static QRect mapPainterQuestionRect(double worldX,
                                    double worldY,
                                    double worldRadius,
                                    double offsetX,
                                    double offsetY,
                                    double scaleFactor,
                                    double treeScale,
                                    double siteScale,
                                    double scale = 3.5,
                                    bool right = true)
{
    int x = static_cast<int>(offsetX + worldX * scaleFactor);
    int y = static_cast<int>(offsetY + worldY * scaleFactor);
    int radius = static_cast<int>(worldRadius * scaleFactor * treeScale);
    int margin = 5; // static_cast<int>(1.0 * scaleFactor);
    int questionRadius = static_cast<int>(scale * scaleFactor * siteScale);

    if (right)
    {
        x = x + radius + margin;
        y = y - questionRadius;
    }
    else
    {
        y = y - radius - margin - (questionRadius * 2);
    }

    return QRect(x, y, questionRadius * 2, questionRadius * 2);
}

static void mapPainterDrawLegend(QPainter &painter, Data &data)
{
    int dw = painter.device()->width();
    int dh = painter.device()->height();
    //int d = (dw < dh) ? dw : dh;
    //int fontPointSize = d / 30; //28;
    int d = (dh > dw) ? dh : dw;
    int fontPointSize = d / 50;
    QFont font("Arial", fontPointSize);
    font.setItalic(true);
    QFontMetrics fm(font);
    painter.setFont(font);

    const auto &speciesSite = data.speciesSite();

    int margin = 10;
    int marginTop = 0;
    if (data.view().moveMapTreeSpeciesLegend())
    {
        marginTop = static_cast<int>(static_cast<double>(fontPointSize) * 5);
    }
    int legendBoxSize = fontPointSize + 2;
    int legendHeight = fontPointSize;
    int legendSpacing = 6;

    int legendH = speciesSite.size() * (legendHeight + legendSpacing);
    int legendW = 0;
    for (const auto &[name, species] : speciesSite)
    {
        int w = fm.horizontalAdvance(name);
        if (w > legendW)
        {
            legendW = w;
        }
    }
    legendW += legendBoxSize + 8;
    
    painter.setPen(Qt::NoPen);
    QColor bgrColor(255, 255, 255, 217);
    painter.setBrush(bgrColor);
    painter.drawRect(margin / 2,
                     margin / 2 + marginTop,
                     legendW + margin, legendH + margin);

    painter.setPen(Qt::black);

    int i = 0;
    for (const auto &[name, species] : speciesSite)
    {
        int y = margin + marginTop + i * (legendHeight + legendSpacing);

        // color box
        QRect boxRect(margin,
                      y + ((legendHeight - legendBoxSize) / 2),
                      legendBoxSize,
                      legendBoxSize);
        painter.setBrush(species.color());
        painter.drawRect(boxRect);

        // text label
        painter.setBrush(Qt::NoBrush);
        painter.drawText(
            margin + legendBoxSize + 8,
            y + legendHeight,
            name
        );

        i++;
    }
}

static void mapPainterDrawGrid(QPainter &painter,
                               double siteWidth,
                               double siteHeight,
                               double offsetX,
                               double offsetY,
                               double scaleFactor,
                               double scaledWidth,
                               double scaledHeight)
{
    QPen gridPen(QColor("#999999"));
    gridPen.setWidth(1);  // Always 1-pixel thick
    painter.setPen(gridPen);

    // Draw vertical grid lines
    int gridStep = 10;
    int gridWidth = static_cast<int>(siteWidth);

    if (gridWidth < 500)
    {
        for (int x = 0; x <= gridWidth; ++x)
        {
            if (x % gridStep != 0 && x > 0 && x < gridWidth)
            {
                continue;
            }
            int px = static_cast<int>(offsetX + x * scaleFactor);
            painter.drawLine(px, static_cast<int>(offsetY), px, static_cast<int>(offsetY + scaledHeight));
        }
    }

    // Draw horizontal grid lines
    int gridHeight = static_cast<int>(siteHeight);
    if (gridHeight < 500)
    {
        for (int y = 0; y <= gridHeight; ++y)
        {
            if (y % gridStep != 0 && y > 0 && y < gridHeight)
            {
                continue;
            }
            int py = static_cast<int>(offsetY + y * scaleFactor);
            painter.drawLine(static_cast<int>(offsetX), py, static_cast<int>(offsetX + scaledWidth), py);
        }
    }
}

MapPainter::MapPainter()
{
    // empty
}

void MapPainter::draw(QPainter &painter, Data &data)
{
    double siteWidth = data.site().width();
    double siteHeight = data.site().height();
    double siteScale = std::sqrt((siteWidth * siteHeight) / 10000.0);

    // Compute scale factor (pixels per meter)
    double ratio;
    if (data.view().width() < data.view().height())
    {
        ratio = data.view().width() / siteWidth;
    }
    else
    {
        ratio = data.view().height() / siteHeight;
    }

    double scaleFactor = ratio * data.view().zoom();
    double ratioInitial = ratio * data.view().zoomMin();

    // Compute scaled world dimensions
    double scaledWidth = siteWidth * scaleFactor;
    double scaledHeight = siteHeight * scaleFactor;

    // Compute translation offsets to center the image
    double shiftX = (data.view().width() - siteWidth * ratioInitial) / 2.0;
    double shiftY = (data.view().height() - siteHeight * ratioInitial) / 2.0;
    double offsetX = data.view().translatationX() * scaleFactor + shiftX;
    double offsetY = data.view().translatationY() * scaleFactor + shiftY;

#if defined(FORDIL_DEBUG_MAP_PAINTER)
    qDebug() << "image w" << data.view().width() << "h" << data.view().height() << "zoom" << data.view().zoom();
    qDebug() << "site w" << siteWidth << "h" << siteHeight;
    qDebug() << "scaled w" << scaledWidth << "h" << scaledHeight << "r" << ratio << "sf" << scaleFactor;
    qDebug() << "shift x" << shiftX << "y" << shiftY;
    qDebug() << "translation x" << data.view().translatationX() << "y" << data.view().translatationY();
    qDebug() << "A x" << offsetX << "y" << offsetY;
    qDebug() << "B x" << (offsetX + siteWidth * scaleFactor) << "y" << offsetY;
    qDebug() << "C x" << offsetX << "y" << (offsetY + siteHeight * scaleFactor);
    qDebug() << "D x" << (offsetX + siteWidth * scaleFactor) << "y" << (offsetY + siteHeight * scaleFactor);
#endif

    painter.resetTransform();  // Ensure we're drawing in pixel coordinates

    // Draw grid
    mapPainterDrawGrid(painter,
                       siteWidth,
                       siteHeight,
                       offsetX,
                       offsetY,
                       scaleFactor,
                       scaledWidth,
                       scaledHeight);

    // Hotspots
    std::map<uint64_t, QColor> hotspotColorTreeMap;
    std::set<uint64_t> hotspotIdCorrectCut;
    TreeSelectionReasonValue hotspotsReason = data.view().hotspotsReason();
    if (hotspotsReason != TreeSelectionReasonValue::None)
    {
        for (const auto &[key, hotspot] : data.hotspots())
        {
            TreeSelectionReasonValue hotspotReason = hotspot.reason();
            if (hotspotReason != hotspotsReason)
            {
                continue;
            }

            bool correctCut = true;

            std::map<uint64_t, bool> cutResults = hotspot.cutResults(data.trees());
            for (const auto &it : cutResults)
            {
                QColor hotspotCutColor(Qt::green);
                if (!it.second)
                {
                    hotspotCutColor = Qt::red;
                    correctCut = false;
                }

                hotspotColorTreeMap[it.first] = hotspotCutColor;
            }

            if (correctCut)
            {
                hotspotIdCorrectCut.insert(hotspot.hotspotId());
            }
        }
    }

    // Spots
    std::map<uint64_t, QColor> spotColorMap;
    QString spotsReason = data.view().valuationReasonText();
    uint64_t spotsReasonId = 0;
    if (spotsReason != "None")
    {
        bool ok{false};
        spotsReasonId = 1; // Show all
    }

    if (spotsReasonId > 0)
    {
        for (const auto &[key, spot] : data.spots())
        {
            int score = data.travailloscopeQuiz().score(spot.spotId());
            QColor scoreColor(data.travailloscopeQuiz().scoreColor(score));

            spotColorMap[spot.spotId()] = scoreColor;
        }
    }

    // Marteloscope tree
    painter.setFont(QFont("Arial", 32));
    painter.setPen(Qt::black);

    double treeIdStartX = -1.0;
    double treeIdStartY = -1.0;
    double treeIdStartRadius = 1.0;

    double treeScale = data.view().treeScale();

    for (const auto &treeId : data.treeIdsSortedByDbh())
    {
        const auto &tree = data.trees().at(treeId);
        const auto &species = data.species().at(tree.speciesId());
        const auto &treeColor = species.color();

        QColor treeQColor;
        if (tree.selectionStatus() == SelectionStatus::Highlighted ||
            tree.selectionStatus() == SelectionStatus::Cross)
        {
            treeQColor.setRgb(255, 255, 0);
        }
        else
        {
            treeQColor.setRgb(treeColor.red(), treeColor.green(), treeColor.blue());
        }

        painter.setBrush(QBrush(treeQColor));

        // Convert world coordinates to pixel space
        double worldX = tree.x();
        double worldY = tree.y();
        double worldRadius = tree.dbh() * 0.5;
        int pixelX = static_cast<int>(offsetX + worldX * scaleFactor);
        int pixelY = static_cast<int>(offsetY + worldY * scaleFactor);
        int pixelRadius = static_cast<int>(worldRadius * scaleFactor * treeScale);

        if (!data.loadedDiscoveryPart() && tree.option() == "start")
        {
            treeIdStartX = tree.x();
            treeIdStartY = tree.y();
            treeIdStartRadius = worldRadius;
        }

        // Draw the ellipse
        QPen objectPen;
        objectPen.setColor(Qt::black);
        objectPen.setWidth(1);

        auto hotspotTreeIt = hotspotColorTreeMap.find(tree.treeId());
        if (hotspotTreeIt != hotspotColorTreeMap.end())
        {
            objectPen.setColor(hotspotTreeIt->second);
            objectPen.setWidth(6);
        }

        painter.setPen(objectPen);
        painter.drawEllipse(QPoint(pixelX, pixelY), pixelRadius, pixelRadius);

        // Draw selection mark when a tree is selected
        if (tree.isSelected() || tree.selectionStatus() == SelectionStatus::Cross)
        {
            QPen selectedPen(Qt::red);
            selectedPen.setWidth(5);
            painter.setPen(selectedPen);
            painter.drawLine(pixelX - pixelRadius, pixelY - pixelRadius, pixelX + pixelRadius, pixelY + pixelRadius);
            painter.drawLine(pixelX + pixelRadius, pixelY - pixelRadius, pixelX - pixelRadius, pixelY + pixelRadius);
        }

        // Draw the tree ID centered within a circular region around the tree location

        // Set up the pen for drawing text
        QPen textPen;
        textPen.setColor(Qt::black);
        textPen.setWidth(1);
        painter.setPen(textPen);

        // Define the rectangle centered at the tree position with dimensions based on pixelRadius
        QRect rect(pixelX - pixelRadius,
                   pixelY - pixelRadius,
                   pixelRadius * 2,
                   pixelRadius * 2);

        // Convert the tree ID to a string
        QString treeIdText = QString::number(tree.treeId());

        // Determine the optimal font size to fit the text within the rectangle
        int bestFontSize = findOptimalFontSize(painter, treeIdText, rect);

        // Set the painter's font to the calculated size
        QFont font = painter.font();
        font.setPointSize(bestFontSize);
        painter.setFont(font);

        // Draw the text centered within the defined rectangle
        painter.drawText(rect, Qt::AlignCenter, treeIdText);
    }

    // Start
    if (treeIdStartX > 0.0 && treeIdStartY > 0.0)
    {
        QImage startImage(":/images/ui_start_flag.png");

        QRect rect = mapPainterQuestionRect(treeIdStartX,
                                            treeIdStartY,
                                            treeIdStartRadius,
                                            offsetX, offsetY,
                                            scaleFactor, treeScale, siteScale,
                                            4.0, false);

        painter.drawImage(rect, startImage, startImage.rect());
    }

    // Travailloscope Spot
    for (const auto &[key, spot] : data.spots())
    {
        // Convert world coordinates to pixel space
        int pixelX = static_cast<int>(offsetX + spot.x() * scaleFactor);
        int pixelY = static_cast<int>(offsetY + spot.y() * scaleFactor);
        int pixelRadius = static_cast<int>(spot.radius() * scaleFactor * treeScale);

        // Draw the ellipse
        if (spot.selectionStatus() == SelectionStatus::Highlighted)
        {
            painter.setBrush(QBrush(QColor("#ffff00")));
        }
        else if (data.travailloscopeQuiz().isFinished(spot.spotId()))
        {
            painter.setBrush(QBrush(QColor("#a4b8d3")));
        }
        else
        {
            painter.setBrush(QBrush(QColor("#ccc")));
        }

        QPen objectPen;
        objectPen.setColor(Qt::black);
        objectPen.setWidth(1);

        auto spotColorIt = spotColorMap.find(spot.spotId());
        if (spotColorIt != spotColorMap.end())
        {
            objectPen.setColor(spotColorIt->second);
            objectPen.setWidth(6);
        }

        painter.setPen(objectPen);

        painter.drawEllipse(QPoint(pixelX, pixelY), pixelRadius, pixelRadius);

        // Text
        // Set up the pen for drawing text
        QPen textPen;
        textPen.setColor(Qt::black);
        textPen.setWidth(1);
        painter.setPen(textPen);

        // Define the rectangle centered at the spot position
        QRect rect(pixelX - pixelRadius,
                   pixelY - pixelRadius,
                   pixelRadius * 2,
                   pixelRadius * 2);

        // Convert the spot ID to a string
        QString spotIdText = QString::number(spot.spotId());

        // Determine the optimal font size to fit the text within the rectangle
        int bestFontSize = findOptimalFontSize(painter, spotIdText, rect);

        // Set the painter's font to the calculated size
        QFont font = painter.font();
        font.setPointSize(bestFontSize);
        painter.setFont(font);

        // Draw the text centered within the defined rectangle
        painter.drawText(rect, Qt::AlignCenter, spotIdText);
    }

    // Travailloscope Spot ?
    if (spotsReasonId > 0)
    {
        QImage spotGreenImage(":/images/ui_question_green.png");
        QImage spotYellowImage(":/images/ui_question_yellow.png");
        QImage spotBlackImage(":/images/ui_question_black.png");

        for (const auto &[key, spot] : data.spots())
        {
            QRect rect = mapPainterQuestionRect(spot.x(),
                                                spot.y(),
                                                spot.radius(),
                                                offsetX, offsetY,
                                                scaleFactor, treeScale, siteScale);

            if (data.travailloscopeQuiz().score(spot.spotId()) == 2)
            {
                painter.drawImage(rect, spotGreenImage, spotGreenImage.rect());
            }
            else if (data.travailloscopeQuiz().score(spot.spotId()) == 1)
            {
                painter.drawImage(rect, spotYellowImage, spotYellowImage.rect());
            }
            else
            {
                painter.drawImage(rect, spotBlackImage, spotBlackImage.rect());
            }
        }
    }

    // Discovery stop
    uint64_t discoveryTreeId = data.discoveryTreeId();
    if (discoveryTreeId > 0)
    {
        QImage informationImage(":/images/ui_question.png");

        auto itTree = data.trees().find(discoveryTreeId);
        if (itTree != data.trees().end())
        {
            QRect rect = mapPainterQuestionRect(itTree->second.x(),
                                                itTree->second.y(),
                                                itTree->second.dbh() * 0.5,
                                                offsetX, offsetY,
                                                scaleFactor, treeScale, siteScale);

            painter.drawImage(rect, informationImage, informationImage.rect());
        }
    }

    // Marteloscope Hotspot
    if (hotspotsReason != TreeSelectionReasonValue::None)
    {
        //QImage informationImage(":/images/ui_question.png");
        QImage spotGreenImage(":/images/ui_question_green.png");
        QImage spotRedImage(":/images/ui_question_red.png");

        double longitudeMin = data.site().longitudeMin();
        double longitudeMax = data.site().longitudeMax();
        double longitudeRange = longitudeMax - longitudeMin;

        double latitudeMin = data.site().latitudeMin();
        double latitudeMax = data.site().latitudeMax();
        double latitudeRange = latitudeMax - latitudeMin;

        for (const auto &[key, hotspot] : data.hotspots())
        {
            TreeSelectionReasonValue hotspotReason = hotspot.reason();
            if (hotspotReason != hotspotsReason)
            {
                continue;
            }

            // Hotspot location
            double longitude = hotspot.longitude();
            double longitude01 = (longitude - longitudeMin) / longitudeRange;

            double latitude = hotspot.latitude();
            double latitude01 = (latitudeMax - latitude) / latitudeRange;

            // Convert world coordinates to pixel space
            double worldX = longitude01 * siteWidth;
            double worldY = latitude01 * siteHeight;

            QRect rect = mapPainterQuestionRect(worldX,
                                                worldY,
                                                hotspot.radius(),
                                                offsetX, offsetY,
                                                scaleFactor, treeScale, siteScale);

#if defined(FORDIL_DEBUG_MAP_PAINTER)
            qDebug() << "hotspot id" << hotspot.hotspotId();
            qDebug() << "longitude" << longitude << "," << longitude01 << "," << worldX << "," << x;
            qDebug() << "latitude" << latitude << "," << latitude01 << "," << worldY << "," << y;
#endif

            if (hotspotIdCorrectCut.count(hotspot.hotspotId()) > 0)
            {
                painter.drawImage(rect, spotGreenImage, spotGreenImage.rect());
            }
            else
            {
                painter.drawImage(rect, spotRedImage, spotRedImage.rect());
            }
        }
    }

    // Location
    if (data.view().isLocationAvailable())
    {
        double longitude = data.view().longitude();
        double longitudeMin = data.site().longitudeMin();
        double longitudeMax = data.site().longitudeMax();
        double longitudeD = longitudeMax - longitudeMin;

        double latitude = data.view().latitude();
        double latitudeMin = data.site().latitudeMin();
        double latitudeMax = data.site().latitudeMax();
        double latitudeD = latitudeMax - latitudeMin;

        if (longitudeD > 1e-6 && latitudeD > 1e-6)
        {
            double longitudeR = (longitude - longitudeMin) / longitudeD;
            double latitudeR = (latitudeMax - latitude) / latitudeD; // Invert

            double longitudeX = siteWidth * longitudeR;
            double latitudeY = siteHeight * latitudeR;

            int locationX = static_cast<int>(offsetX + longitudeX * scaleFactor);
            int locationY = static_cast<int>(offsetY + latitudeY * scaleFactor);

            int w = 150;
            int w2 = w / 2;

            const QPointF topLeft(locationX - w2, locationY - w2);
            const QSizeF targetSize(w, w);
            const QRectF targetRect(topLeft, targetSize);

            if (data.view().isAzimuthAvailable())
            {
                painter.save();
                painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

                QImage locationImage(":/images/ui_gps_location_cone.png");

                const QPointF c = targetRect.center();
                painter.translate(c);
                painter.rotate(data.view().azimuth());
                painter.translate(-c);

                painter.drawImage(targetRect, locationImage, locationImage.rect());

                painter.restore();
            }
            else
            {
                QImage locationImage(":/images/ui_gps_location.png");
                painter.drawImage(targetRect, locationImage, locationImage.rect());
            }
        }
    }

    // Legend
    if (data.view().showMapTreeSpeciesLegend() &&
        data.loadedSystem() == "Marteloscope")
    {
        mapPainterDrawLegend(painter, data);
    }

    // Finish painting
    painter.end();
}

void MapPainter::click(SelectedTree &selectedTree,
                       SelectedHotspot &selectedHotspot,
                       SelectedSpot &selectedSpot,
                       SelectedStop &selectedStop,
                       SelectedPoint &selectedPoint,
                       Data &data,
                       double rx,
                       double ry)
{
    double siteWidth = data.site().width();
    double siteHeight = data.site().height();
    double siteScale = std::sqrt((siteWidth * siteHeight) / 10000.0);

    // Compute scale factor (pixels per meter)
    double ratio;
    if (data.view().width() < data.view().height())
    {
        ratio = data.view().width() / siteWidth;
    }
    else
    {
        ratio = data.view().height() / siteHeight;
    }

    double scaleFactor = ratio * data.view().zoom();
    double ratioInitial = ratio * data.view().zoomMin();

    // Compute scaled world dimensions
    double scaledWidth = siteWidth * scaleFactor;
    double scaledHeight = siteHeight * scaleFactor;

    // Compute translation offsets to center the image
    double shiftX = (data.view().width() - siteWidth * ratioInitial) / 2.0;
    double shiftY = (data.view().height() - siteHeight * ratioInitial) / 2.0;
    double offsetX = data.view().translatationX() * scaleFactor + shiftX;
    double offsetY = data.view().translatationY() * scaleFactor + shiftY;

    double treeScale = data.view().treeScale();

    // Pointer coordinates
    double mx = rx * data.view().width();
    double my = ry * data.view().height();

    int mxInt = static_cast<int>(mx);
    int myInt = static_cast<int>(my);

    // Pick a point
    double mxm = (mx - offsetX) / scaleFactor;
    double mym = (my - offsetY) / scaleFactor;

    double mxr = mxm / siteWidth;
    double myr = mym / siteHeight;

    double mxl = (data.site().longitudeMax() - data.site().longitudeMin()) * mxr;
    double myl = (data.site().latitudeMax() - data.site().latitudeMin()) * myr;

    double latitude = data.site().latitudeMax() - myl;
    double longitude = data.site().longitudeMin() + mxl;

    selectedPoint.pick(latitude, longitude);

    qDebug() << "gps" << QString::number(longitude, 'f', 8) << QString::number(latitude, 'f', 8);

    // Pick a Marteloscope tree
    uint64_t pickedTreeId = UINT64_MAX; // Nothing is picked initially

    TreeSelectionReasonValue hotspotsReason = data.view().hotspotsReason();
    if (hotspotsReason == TreeSelectionReasonValue::None &&
        !data.loadedDiscoveryPart())
    {
        // Iterate all trees in drawing order
        for (const auto &treeId : data.treeIdsSortedByDbh())
        {
            auto &tree = data.trees().at(treeId);

            // Reset pre-selection in all trees
            tree.setSelectionStatus(SelectionStatus::None);

            // Convert world coordinates to pixel space
            double worldX = tree.x();
            double worldY = tree.y();
            double x = offsetX + worldX * scaleFactor;
            double y = offsetY + worldY * scaleFactor;
            double worldRadius = tree.dbh() * 0.5; // Circle radius [m]
            double radius = worldRadius * scaleFactor * treeScale;

            // Set picked tree ID when [mx, my] is inside tree circle, last wins
            double dx = (mx - x) * (mx - x);
            double dy = (my - y) * (my - y);
            double d = std::sqrt(dx + dy);
            if (d < radius)
            {
                pickedTreeId = tree.treeId();
            }
        }
    }

    if (pickedTreeId != UINT64_MAX)
    {
        // The tree with pickedTreeId was picked
        auto &tree = data.trees().at(pickedTreeId);
        const auto &species = data.species().at(tree.speciesId());

        // Make the picked tree interactive
        tree.setSelectionStatus(SelectionStatus::Cross);

        // Populate result with picked object info
        selectedTree.pick(tree, species);
    }
    else
    {
        // Nothing was picked
        selectedTree.reset();
    }

    // Pick a Travailloscope spot
    uint64_t pickedSpotId = UINT64_MAX; // Nothing is picked initially

    QString spotsReason = data.view().valuationReasonText();
    uint64_t spotsReasonId = 0;
    if (spotsReason != "None")
    {
        bool ok{false};
        spotsReasonId = 1; // Show all -> allow to click on any (?)
    }

    if (spotsReasonId > 0)
    {
        // Pick ?
        for (const auto &[key, spot] : data.spots())
        {
            QRect rect = mapPainterQuestionRect(spot.x(),
                                                spot.y(),
                                                spot.radius(),
                                                offsetX, offsetY,
                                                scaleFactor, treeScale, siteScale);

            // Is cursor inside the ? rectangle
            if (rect.contains(mxInt, myInt))
            {
                pickedSpotId = spot.spotId();
                break;
            }
        }
    }
    else
    {
        for (const auto &[key, spot] : data.spots())
        {
            // Convert world coordinates to pixel space
            int x = static_cast<int>(offsetX + spot.x() * scaleFactor);
            int y = static_cast<int>(offsetY + spot.y() * scaleFactor);
            int radius = static_cast<int>(spot.radius() * scaleFactor * treeScale);

            double dx = (mx - x) * (mx - x);
            double dy = (my - y) * (my - y);
            double d = std::sqrt(dx + dy);
            if (d < radius)
            {
                pickedSpotId = spot.spotId();
                break;
            }
        }
    }

    if (pickedSpotId != UINT64_MAX)
    {
        // The spot with pickedSpotId was picked
        auto &spot = data.spots().at(pickedSpotId);

        // Populate result with picked object info
        selectedSpot.pick(spot);
    }
    else
    {
        // Nothing was picked
        selectedSpot.reset();
    }

    // Pick a Discovery stop
    uint64_t pickedStopId = UINT64_MAX; // Nothing is picked initially

    uint64_t discoveryTreeId = data.discoveryTreeId();
    if (discoveryTreeId > 0)
    {
        auto itTree = data.trees().find(discoveryTreeId);
        if (itTree != data.trees().end())
        {
            QRect rect = mapPainterQuestionRect(itTree->second.x(),
                                                itTree->second.y(),
                                                itTree->second.dbh() * 0.5,
                                                offsetX, offsetY,
                                                scaleFactor, treeScale, siteScale);

            // Is cursor inside the rectangle
            if (rect.contains(mxInt, myInt))
            {
                pickedStopId = discoveryTreeId;
            }
        }
    }

    if (pickedStopId != UINT64_MAX)
    {
        uint64_t discoveryState = static_cast<uint64_t>(data.discoveryState());

        // Populate result with picked object info
        selectedStop.pick(discoveryState);
    }
    else
    {
        // Nothing was picked
        selectedStop.reset();
    }

    // Pick a  Marteloscope Hotspot
    uint64_t pickedHotspotId = UINT64_MAX; // Nothing is picked initially

    if (hotspotsReason != TreeSelectionReasonValue::None)
    {
        double longitudeMin = data.site().longitudeMin();
        double longitudeMax = data.site().longitudeMax();
        double longitudeRange = longitudeMax - longitudeMin;

        double latitudeMin = data.site().latitudeMin();
        double latitudeMax = data.site().latitudeMax();
        double latitudeRange = latitudeMax - latitudeMin;

        for (const auto &[key, hotspot] : data.hotspots())
        {
            TreeSelectionReasonValue hotspotReason = hotspot.reason();
            if (hotspotReason != hotspotsReason)
            {
                continue;
            }

            // Hotspot location
            double longitude = hotspot.longitude();
            double longitude01 = (longitude - longitudeMin) / longitudeRange;

            double latitude = hotspot.latitude();
            double latitude01 = (latitudeMax - latitude) / latitudeRange;

            // Convert world coordinates to pixel space
            double worldX = longitude01 * siteWidth;
            double worldY = latitude01 * siteHeight;

            QRect rect = mapPainterQuestionRect(worldX,
                                                worldY,
                                                hotspot.radius(),
                                                offsetX, offsetY,
                                                scaleFactor, treeScale, siteScale);

            // Is cursor inside the ? rectangle
            if (rect.contains(mxInt, myInt))
            {
                pickedHotspotId = hotspot.hotspotId();
            }
        }
    }

    if (pickedHotspotId != UINT64_MAX)
    {
        // The hotspot with pickedHotspotId was picked
        for (const auto &[key, hotspot] : data.hotspots())
        {
            if (hotspot.hotspotId() == pickedHotspotId)
            {
                // Populate result with picked object info
                selectedHotspot.pick(hotspot);
                break;
            }
        }
    }
    else
    {
        // Nothing was picked
        selectedHotspot.reset();
    }
}
