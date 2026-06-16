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

#include "c_data.h"
#include <random>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QVariant>
#include <QRegularExpression>

// Print warning messages when site data are not consistent
// #define FORDIL_DATA_CHECK_DEBUG 1

static const std::vector<QString> ColorPaletteMPN65 = {
    "#ff0029", "#377eb8", "#66a61e", "#984ea3", "#00d2d5", "#ff7f00", "#af8d00",
    "#7f80cd", "#b3e900", "#c42e60", "#a65628", "#f781bf", "#8dd3c7", "#bebada",
    "#fb8072", "#80b1d3", "#fdb462", "#fccde5", "#bc80bd", "#ffed6f", "#c4eaff",
    "#cf8c00", "#1b9e77", "#d95f02", "#e7298a", "#e6ab02", "#a6761d", "#0097ff",
    "#00d067", "#000000", "#252525", "#525252", "#737373", "#969696", "#bdbdbd",
    "#f43600", "#4ba93b", "#5779bb", "#927acc", "#97ee3f", "#bf3947", "#9f5b00",
    "#f48758", "#8caed6", "#f2b94f", "#eff26e", "#e43872", "#d9b100", "#9d7a00",
    "#698cff", "#d9d9d9", "#00d27e", "#d06800", "#009f82", "#c49200", "#cbe8ff",
    "#fecddf", "#c27eb6", "#8cd2ce", "#c4b8d9", "#f883b0", "#a49100", "#f48800",
    "#27d0df", "#a04a9b"};

static double randomDouble(double max)
{
    static std::random_device rd;  // Seed
    static std::mt19937 gen(rd()); // Random number engine
    std::uniform_real_distribution<> dis(0.0, max);
    return dis(gen);
}

static bool openFile(QFile &file, const QString &path)
{
    if (QFile::exists(path))
    {
#if defined(FORDIL_DEBUG_DATA)
        qDebug() << "File" << path << "exists";
#endif
    }
    else
    {
        qCritical() << "File" << path << "not found!";
        return false;
    }

    file.setFileName(path);

    if (!file.open(QIODevice::ReadOnly))
    {
        qCritical() << "Cannot open file" << path;
        return false;
    }

#if defined(FORDIL_DEBUG_DATA)
    qDebug() << "File" << path << "is open";
#endif

    return true;
}

Data::Data()
{
    // empty
}

static QString xlsxCellToString(QXlsx::Document &xlsx, int r, int c)
{
    QVariant value = xlsx.read(r, c);
    if (value.isValid())
    {
        return value.toString().trimmed();
    }

    return "";
}

static Table xlsxToTable(QXlsx::Document &xlsx,
                         const QString &sheet,
                         bool mandatory = false)
{
    Table table;

    // Select sheet
    bool sheetFound = xlsx.selectSheet(sheet);

    // Check mandatory sheet
    if (!sheetFound)
    {
        if (mandatory)
        {
            throw std::runtime_error("Mandatory sheet '" + sheet.toStdString() + "' not found");
        }
        else
        {
            return table;
        }
    }

    // XLSX range
    QXlsx::CellRange range = xlsx.dimension();

    // Determine used range for table values
    int lastValueColumn = 0;
    for (int c = range.firstColumn(); c <= range.lastColumn(); ++c)
    {
        QString columnHeader = xlsxCellToString(xlsx, range.firstRow(), c);
        if (columnHeader.isEmpty())
        {
            break;
        }

        lastValueColumn = c;
    }

    int firstValueRow = range.firstRow() + 1;
    int lastValueRow = 0;
    for (int r = firstValueRow; r <= range.lastRow(); ++r)
    {
        bool hasValue{false};
        for (int c = range.firstColumn(); c <= lastValueColumn; ++c)
        {
            QString text = xlsxCellToString(xlsx, r, c);
            if (!text.isEmpty())
            {
                hasValue = true;
                break;
            }
        }

        if (!hasValue)
        {
            break;
        }

        lastValueRow = r;
    }

    size_t columnValueLength = 0;
    if (lastValueRow >= firstValueRow)
    {
        columnValueLength = static_cast<size_t>(lastValueRow - firstValueRow);
        columnValueLength++;
    }

#if defined(FORDIL_DEBUG_DATA)
    qDebug() << "xlsx sheet" << sheet
             << "lastValueColumn" << lastValueColumn
             << "lastValueRow" << lastValueRow
             << "columnValueLength" << columnValueLength;
#endif

    // Read values
    for (int c = range.firstColumn(); c <= lastValueColumn; ++c)
    {
        QString columnHeader = xlsxCellToString(xlsx, range.firstRow(), c);
        columnHeader = columnHeader.toLower();
        columnHeader.replace("_", " ");

        std::vector<QString> columnValues;
        columnValues.reserve(columnValueLength);
        for (int r = firstValueRow; r <= lastValueRow; ++r)
        {
            columnValues.push_back(xlsxCellToString(xlsx, r, c));
        }

        table.insert(columnHeader, columnValues);
    }

#if defined(FORDIL_DEBUG_DATA)
    qDebug() << "xlsx sheet" << sheet
             << "has" << QString::number(table.colCount()) << "columns"
             << "and" << QString::number(table.rowCount()) << "rows";
#endif

    return table;
}

void Data::loadSite(QXlsx::Document &xlsx, const QString &siteName, const QString &system)
{
    // Read all data lines
    Table table = xlsxToTable(xlsx, "Site", true);

    // Process
    if (!table.hasColumn("area [ha]"))
    {
        throw std::runtime_error("Cannot read site Ha");
    }
    double surfaceHa = table.toDouble("area [ha]", 0);

    double northAngle = table.toDouble("north angle", 0);

    m_site = Site(siteName, system, 10.0, 10.0, surfaceHa, northAngle);
    m_view.setSiteDimensions(m_site.width(), m_site.height());

    // Site map
    // QXlsx::Worksheet *worksheet = dynamic_cast<QXlsx::Worksheet*>(xlsx.sheet("Site"));
    // if (worksheet)
    // {
    //     QList<QXlsx::Drawing*> drawings = worksheet->drawings();
    //     for (QXlsx::Drawing *d : drawings)
    //     {
    //         if (auto img = dynamic_cast<QXlsx::DrawingPicture*>(d))
    //         {
    //             m_siteMapImage = img->image();
    //         }
    //     }
    // }
}

void Data::loadIntroduction(QXlsx::Document &xlsx)
{
    // Read all data lines
    Table table = xlsxToTable(xlsx, "Introduction");

    // Process
    if (table.rowCount() < 1)
    {
        return;
    }

    m_introduction = table.text("text", 0);

    core().insertTranslation(m_introduction, table.text("text cs", 0), "cs");
    core().insertTranslation(m_introduction, table.text("text fr", 0), "fr");
}

void Data::loadSpecies(QXlsx::Document &xlsx)
{
    // Read all data lines
    Table table = xlsxToTable(xlsx, "Species", true);

    // Process
    uint64_t speciesId = 1;
    for (size_t i = 0; i < table.rowCount(); i++)
    {
        // Species_full
        QString fullName = table.text("species", i);

        // Color
        QColor color(ColorPaletteMPN65[i]);
        auto colorCell = xlsx.cellAt(i + 2, 2);
        if (colorCell)
        {
            QXlsx::Format fmt = colorCell->format();
            if (fmt.fillPattern() != QXlsx::Format::PatternNone)
            {
                color = fmt.patternBackgroundColor();
#if defined(FORDIL_DEBUG_DATA)
                qDebug() << "species" << i << "name" << fullName
                         << "color" << color.name()
                         << "text" << colorCell->value().toString();
#endif
            }
            else
            {
                qWarning() << "Species" << i << "name" << fullName << "no pattern";
            }
        }
        else
        {
            qWarning() << "Species" << i << "name" << fullName << "no color cell";
        }

        // Price
        std::map<QString, double> price;
        price["A"] = table.toDouble("price a", i);
        price["B"] = table.toDouble("price b", i);
        price["C"] = table.toDouble("price c", i);
        price["D"] = table.toDouble("price d", i);
        price["FW"] = table.toDouble("price fw", i);

        // Add new species
        Species species(speciesId, fullName, color, price);
        m_species.insert_or_assign(speciesId, std::move(species));
        m_speciesFullName.insert_or_assign(fullName.toStdString(), speciesId);
        speciesId++;
    }
}

void Data::loadEcologicalValues(QXlsx::Document &xlsx)
{
    // Read all data lines
    Table table = xlsxToTable(xlsx, "Ecovalue", false);

    // Process
    for (size_t i = 0; i < table.rowCount(); i++)
    {
        uint64_t ecologicalValueId =  table.toULongLong("code", i);
        QString theme = table.text("theme", i);
        QString description = table.text("code txt", i);
        double nat    = table.toDouble("nat", i);
        double chiro  = table.toDouble("chiro", i);
        double avri   = table.toDouble("avri", i);
        double ento   = table.toDouble("ento", i);
        double global = table.toDouble("global", i);

        // Add
        EcologicalValue v(ecologicalValueId,
                          theme,
                          description,
                          nat,
                          chiro,
                          avri,
                          ento,
                          global);

        m_ecologicalValues.insert_or_assign(ecologicalValueId, std::move(v));
    }
}

static double dataTreeEcovalueGlob(const QString &ecoValue,
    const std::unordered_map<uint64_t, EcologicalValue> &ecologicalValues)
{
    double result = 0;

    // Ecological value : ecovalue_code [10] : 362 or "371;382"
    std::vector<uint64_t> codes;
    QStringList parts = ecoValue.split(";", Qt::SkipEmptyParts);
    for (QString part : parts)
    {
        bool ok = false;
        uint64_t code = part.toULongLong(&ok);
        if (ok)
        {
            codes.push_back(code);
        }
    }

    for (const auto &code : codes )
    {
        const auto it = ecologicalValues.find(code);
        if (it != ecologicalValues.end())
        {
            result += it->second.global();
        }
    }

    return result;
}

void Data::addTree(uint64_t treeId,
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
{
    // Check tree species
    if (m_species.find(speciesId) == m_species.end())
    {
        qCritical() << "Cannot find speciesId" << speciesId << "for treeId" << treeId;
        return;
    }

    // Insert new tree 
    Tree t(treeId, speciesId, latitude, longitude, gridId,
           dbh, dbh10, height, volumeQual, cv, yGain, option,
           ecovalueGlob, q4Prosilvalue, dead, bio, dmh);

    m_trees.insert_or_assign(treeId, std::move(t));

    // Update DBH sort order
    size_t insertPos = SIZE_MAX;
    size_t n = m_treeIdsSortedByDbh.size();
    for (size_t i = 0; i < n; i++)
    {
        uint64_t id = m_treeIdsSortedByDbh[i];
        if (dbh > m_trees[id].dbh())
        {
            insertPos = i;
            break;
        }
    }

    if (insertPos == SIZE_MAX)
    {
        m_treeIdsSortedByDbh.push_back(treeId);
    }
    else
    {
        m_treeIdsSortedByDbh.resize(n + 1);

        for (size_t i = n; i > insertPos; --i)
        {
            m_treeIdsSortedByDbh[i] = m_treeIdsSortedByDbh[i - 1];
        }

        m_treeIdsSortedByDbh[insertPos] = treeId;
    }
}

void Data::loadTrees(QXlsx::Document &xlsx)
{
    // Read all data lines
    Table table = xlsxToTable(xlsx, "Tree", true);

    // Process
    for (size_t i = 0; i < table.rowCount(); i++)
    {
        // TreeID
        uint64_t treeId = table.toULongLong("treeid", i);

        // Species
        QString speciesStr = table.text("species", i);
        auto it = m_speciesFullName.find(speciesStr.toStdString());
        if (it == m_speciesFullName.end())
        {
            qCritical() << "Cannot find species" << speciesStr << "for treeId" << treeId;
            continue;
        }

        uint64_t speciesId = it->second;
        auto itSpecies = m_species.find(speciesId);
        if (itSpecies == m_species.end())
        {
            qCritical() << "Cannot find species" << speciesStr << "for treeId" << treeId;
            continue;
        }

        if (m_speciesSite.count(speciesStr) < 1)
        {
            m_speciesSite[speciesStr] = itSpecies->second;
        }

        // Coordinates
        QString latitudeStr = table.text("wgs lat n", i);
        QString longitudeStr = table.text("wgs lon e", i);

        if (latitudeStr.isEmpty() || longitudeStr.isEmpty())
        {
            qCritical() << "No location for treeId" << treeId;
            continue;
        }

        bool ok = false;
        double latitude = latitudeStr.replace(",", ".").toDouble(&ok);
        double longitude = longitudeStr.replace(",", ".").toDouble(&ok);

        // Grid
        size_t gridId = static_cast<size_t>(table.toULongLong("grid", i));

        // Health and bio
        bool dead = (table.toInt("dead", i) > 0);
        bool bio = (table.toInt("bio", i) > 0);

        // DBH
        double dbh = table.toDouble("dbh", i) * 0.01; // [m]
        if (dbh < 0.001)
        {
            dbh = 0.001;
        }

        double dbh10 = table.toDouble("dbh10", i) * 0.01;

        // Volume Qual
        std::map<QString, double> volumeQual;
        volumeQual["A"] = table.toDouble("vol a", i);
        volumeQual["B"] = table.toDouble("vol b", i);
        volumeQual["C"] = table.toDouble("vol c", i);
        volumeQual["D"] = table.toDouble("vol d", i);
        volumeQual["FW"] = table.toDouble("vol fw", i);

        double cv = 0;
        for (const auto &[qual, volume] : volumeQual)
        {
            cv += itSpecies->second.cv(qual, volume);
        }

        double yGain = 0;
        yGain += table.toDouble("ygain a", i);
        yGain += table.toDouble("ygain b", i);
        yGain += table.toDouble("ygain c", i);
        yGain += table.toDouble("ygain d", i);
        yGain += table.toDouble("ygain fw", i);

        // Height
        double height = 0;

        // Prosilva
        QString ecoValue = table.text("ecovalue", i);
        double ecovalueGlob = dataTreeEcovalueGlob(ecoValue, m_ecologicalValues);

        double q4Prosilvalue = 0;

        // Option
        QString option;
        if (table.toInt("start", i) > 0)
        {
            option = "start";
        }

        // 0 Cavities
        // 1 Dead wood in the crown
        // 2 Epiphytic, epixylic, parasitic
        // 3 Mushrooms
        // 4 Other ecological elements
        // 5 Standing dead wood
        // 6 Wounds and exposed wood
        std::vector<int> dmh;
        dmh.resize(7);
        dmh[0] = table.toInt("cavities", i);
        dmh[1] = table.toInt("dead wood in the crown", i);
        dmh[2] = table.toInt("epiphytic, epixylic, parasitic", i);
        dmh[3] = table.toInt("mushrooms", i);
        dmh[4] = table.toInt("other ecological elements", i);
        dmh[5] = table.toInt("standing dead wood", i);
        dmh[6] = table.toInt("wounds and exposed wood", i);

        // Add new tree
        addTree(treeId, speciesId, latitude, longitude, gridId,
                dbh, dbh10, height, volumeQual, cv, yGain, option,
                ecovalueGlob, q4Prosilvalue, dead, bio, dmh);
    }

    updateQ4Prosilvalue();
    updateSiteDim();
}

void Data::updateQ4Prosilvalue()
{
    std::vector<Tree*> trees;
    trees.reserve(m_trees.size());

    // collect pointers
    size_t nTrees = 0;
    for (auto& [id, tree] : m_trees)
    {
        trees.push_back(&tree);
        nTrees++;
    }

    // sort descending by ecovalueGlob
    std::sort(trees.begin(), trees.end(),
        [](const Tree* a, const Tree* b) {
            return a->ecovalueGlob() > b->ecovalueGlob();
        }
    );

    // compute top 25%
    double countd = static_cast<double>(nTrees) * 0.25;
    size_t count = static_cast<size_t>(std::round(countd));
    if (count > trees.size())
    {
        count = trees.size();
    }

    // apply
    for (size_t i = 0; i < count; ++i)
    {
        trees[i]->setQ4Prosilvalue(1.0);
    }
}

static double dataLongitudeDegreesPerMeter(double latitudeDegrees)
{
    // ϕ at latitude 50° in radians = ( 50 * π ) / 180 ≈ 0.8727 radians
    //
    // Simple:
    //   1 / ( 111320 * cos(ϕ) )
    //
    // WGS-84 Latitude Formula:
    //   1 / ( ( π / 180 ) * 6378137 * cos(ϕ) * 1 / ( sqrt(1 - 0.00669437999014 * sin(ϕ) * sin(ϕ)) ) )

    constexpr double metersPerDegreeAtEquator = 111320.0;
    double latRad = latitudeDegrees * M_PI / 180.0;
    double result = 1.0 / (metersPerDegreeAtEquator * std::cos(latRad));

    qDebug() << "longitudeDegreesPerMeter at lat"
             << QString::number(latitudeDegrees, 'f', 4)
             << "is" << QString::number(result, 'f', 8);

    return result;
}

static double dataLatitudeDegreesPerMeter()
{
    // Simple:
    //   nearly constant
    //
    // WGS-84 Latitude Formula:
    //   ϕ at latitude 50° in radians = ( 50 * π ) / 180 ≈ 0.8727 radians
    //   1 / ( 111132.92 − 559.82 * cos(2ϕ) + 1.175 * cos(4ϕ) − 0.0023 * cos(6ϕ) )

    double result = 1.0 / 111320.0; // ≈ 0.000008983

    qDebug() << "latitudeDegreesPerMeter" << QString::number(result, 'f', 8);

    return result;
}

void Data::updateSiteDim()
{
    // Calculate minimal and maximal values of tree geographic coordinates.
    // Minimal and maximal values are calculated to determine the site area.
    auto it = m_trees.begin();

    double longitudeMin = 0;
    double longitudeMax = 0;
    double latitudeMin = 0;
    double latitudeMax = 0;

    if (it != m_trees.end())
    {
        longitudeMin = it->second.longitude();
        longitudeMax = it->second.longitude();
        latitudeMin = it->second.latitude();
        latitudeMax = it->second.latitude();

        ++it;
    }

    while (it != m_trees.end())
    {
        if (it->second.longitude() < longitudeMin) longitudeMin = it->second.longitude();
        if (it->second.longitude() > longitudeMax) longitudeMax = it->second.longitude();
        if (it->second.latitude() < latitudeMin) latitudeMin = it->second.latitude();
        if (it->second.latitude() > latitudeMax) latitudeMax = it->second.latitude();

        ++it;
    }

#if defined(FORDIL_DEBUG_DATA)
    qDebug() << "calculated site x longitude min" << longitudeMin << "max" << longitudeMax;
    qDebug() << "calculated site y latitude  min" << latitudeMin << "max" << latitudeMax;
#endif

    double longitudeRange = longitudeMax - longitudeMin;
    double latitudeRange = latitudeMax - latitudeMin;

    // Longitude map area
    double longitudeDegreesPerMeter = dataLongitudeDegreesPerMeter(latitudeMin);

    double longitudeMapMargin = longitudeDegreesPerMeter * 10.0; // Extend calculated area by few meters
    double longitudeMapMin = longitudeMin - longitudeMapMargin;
    double longitudeMapMax = longitudeMax + longitudeMapMargin;
    double longitudeMapRange = longitudeMapMax - longitudeMapMin;

    // Latitude map area
    double latitudeDegreesPerMeter = dataLatitudeDegreesPerMeter();

    double latitudeMapMargin = latitudeDegreesPerMeter * 10.0; // Extend calculated area by few meters
    double latitudeMapMin = latitudeMin - latitudeMapMargin;
    double latitudeMapMax = latitudeMax + latitudeMapMargin;
    double latitudeMapRange = latitudeMapMax -latitudeMapMin;

    // Margin
    m_view.setMargin(0.0);

    // Set site location
    m_site.setLocation(latitudeMapMin, longitudeMapMin,
                       latitudeMapMax, longitudeMapMax,
                       latitudeDegreesPerMeter, longitudeDegreesPerMeter);

    // Set site dimension in meters
    double w = longitudeMapRange / longitudeDegreesPerMeter;
    double h = latitudeMapRange / latitudeDegreesPerMeter;
#if defined(FORDIL_DEBUG_DATA)
    qDebug() << "calculated site w" << w << "h" << h;
#endif
    m_site.setDimension(w, h);
    m_view.setSiteDimensions(w, h);

    // Convert geographic coordinate system to screen coordinate system
    // Bottom-left [ longitude = 0, latitude = 0 ] ->  Top-left [ x = 0, y = 0 ]
    //   direction [ right East   , up           ] -> direction [ right, down  ]
    size_t i = 0;
    double xMin = 0;
    double xMax = 0;
    double yMin = 0;
    double yMax = 0;

    it = m_trees.begin();
    while (it != m_trees.end())
    {
        double x = (it->second.longitude() - longitudeMapMin) / longitudeDegreesPerMeter;
        double y = (latitudeMapMax - it->second.latitude()) / latitudeDegreesPerMeter;

        it->second.setPosition(x, y);

        if (i == 0)
        {
            xMin = x;
            xMax = x;
            yMin = y;
            yMax = y;
        }
        else
        {
            if (x < xMin) xMin = x;
            if (x > xMax) xMax = x;
            if (y < yMin) yMin = y;
            if (y > yMax) yMax = y;
        }

        ++i;
        ++it;
    }

#if defined(FORDIL_DEBUG_DATA)
    qDebug() << "calculated site x min" << xMin << "max" << xMax;
    qDebug() << "calculated site y min" << yMin << "max" << yMax;
#endif

    // https://www.openstreetmap.org Export area
    qDebug() << "site left   :" << QString::number(longitudeMapMin, 'f', 8);
    qDebug() << "site right  :" << QString::number(longitudeMapMax, 'f', 8);
    qDebug() << "site top    :" << QString::number(latitudeMapMax, 'f', 8);
    qDebug() << "site bottom :" << QString::number(latitudeMapMin, 'f', 8);
}

void Data::loadTreeLogs(QXlsx::Document &xlsx)
{
    // Read all data lines
    Table table = xlsxToTable(xlsx, "Log Vol", false);

    // Process
    for (size_t i = 0; i < table.rowCount(); i++)
    {
        uint64_t treeId = table.toULongLong("treeid", i);
        QString qual = table.text("qual", i);
        double vol = table.toDouble("vol", i);
        double cv = table.toDouble("cv", i);
        double yGain = table.toDouble("ygain i", i);

        // Add new tree log
        auto it = m_trees.find(treeId);
        if (it == m_trees.end())
        {
            qCritical() << "Cannot find treeId" << treeId
                        << "for 'Log Vol' row" << i;
            continue;
        }

        TreeLog treeLog(treeId, qual, vol, cv, yGain);
        (void)it->second.insertTreeLog(treeLog);
    }
}

void Data::loadTypology(QXlsx::Document &xlsx)
{
    // Auto calculated sha, grid, scale and predefined DBH from
    size_t nGrids = 1;
    for (auto &[key, tree] : trees())
    {
        if (tree.gridId() > nGrids)
        {
            nGrids = tree.gridId();
        }
    }

    double Sha_tot = m_site.ha();
    double Sha_grid = Sha_tot / static_cast<double>(nGrids);

    double Scale_y = 1.0;
    double North_angle = m_site.northAngle();

    double lw_from = 17.5 * 0.01;  // PB  18 cm - 27 cm  in [m]
    double mw_from = 27.5 * 0.01;  // BM  28 cm - 47 cm
    double bw_from = 47.5 * 0.01;  // GB  48 cm - 67 cm 
    double vbw_from = 67.5 * 0.01; // TGB 68 cm - x     (example 102)

    // Add
    m_typology.setGrid(Sha_grid, Sha_tot, nGrids, Scale_y, North_angle);
    m_typology.setWoodSize(lw_from, mw_from, bw_from, vbw_from, bw_from);
}

void Data::loadExperts(QXlsx::Document &xlsx)
{
    // Read all data lines
    Table table = xlsxToTable(xlsx, "Expert Cut", false);

    std::set<uint64_t> cutTreeIds;

    // Process
    for (size_t i = 0; i < table.rowCount(); i++)
    {
        uint64_t treeId = table.toULongLong("treeid", i);
        if (treeId < 1)
        {
            qCritical() << "Cannot read treeId at row" << i << "for expert cut tree";
            continue;
        }

        if (m_trees.find(treeId) == m_trees.end())
        {
            qCritical() << "Cannot find treeId" << treeId << "for expert cut tree";
            continue;
        }

        // Add
        cutTreeIds.insert(treeId);
    }

    m_typology.setCutTreeIdsExpert(cutTreeIds);
}

void Data::loadHotspots(QXlsx::Document &xlsx)
{
    // Read all data lines
    Table table = xlsxToTable(xlsx, "Hotspot", false);

    // Process
    for (size_t i = 0; i < table.rowCount(); i++)
    {
        uint64_t hotspotId = table.toULongLong("hotspot", i);

        uint64_t treeId = table.toULongLong("treeid", i);
        auto itTree = m_trees.find(treeId);
        if (itTree == m_trees.end())
        {
            qCritical() << "Cannot find treeId" << treeId << "for hotspot";
            continue;
        }

        std::vector<QString> cut(3);
        cut[0] = table.text("cut option a", i);
        cut[1] = table.text("cut option b", i);
        cut[2] = table.text("cut option c", i);

        TreeSelectionReasonValue reason =
            treeSelectionReasonFromString(table.text("reason", i));

        double latitude = itTree->second.latitude();
        double longitude = itTree->second.longitude();
        double radius = itTree->second.dbh() * 0.5;

        QString pedagogicalContents = table.text("pedagogical contents", i);
        core().insertTranslation(pedagogicalContents, table.text("pedagogical contents cs", i), "cs");
        core().insertTranslation(pedagogicalContents, table.text("pedagogical contents fr", i), "fr");

        // Add hotspot
        HotspotTree hotspotTree(hotspotId, treeId, cut, reason, latitude, longitude, radius, "", pedagogicalContents);
        
        auto result = m_hotspotList.insert(std::make_pair(hotspotId, Hotspot(hotspotId)));
        auto &hotspot = result.first->second;

        hotspot.push_back(hotspotTree);
    }

#if defined(FORDIL_DEBUG_DATA)
    qDebug() << "loaded" << m_hotspotList.size() << "hotspots";
#endif
}

void Data::updateSiteDimFromSpots()
{
    // Calculate minimal and maximal values of tree geographic coordinates.
    // Minimal and maximal values are calculated to determine the site area.
    auto it = m_spots.begin();

    double longitudeMin = 0;
    double longitudeMax = 0;
    double latitudeMin = 0;
    double latitudeMax = 0;

    if (it != m_spots.end())
    {
        longitudeMin = it->second.longitude();
        longitudeMax = it->second.longitude();
        latitudeMin = it->second.latitude();
        latitudeMax = it->second.latitude();

        ++it;
    }

    while (it != m_spots.end())
    {
        if (it->second.longitude() < longitudeMin) longitudeMin = it->second.longitude();
        if (it->second.longitude() > longitudeMax) longitudeMax = it->second.longitude();
        if (it->second.latitude() < latitudeMin) latitudeMin = it->second.latitude();
        if (it->second.latitude() > latitudeMax) latitudeMax = it->second.latitude();

        ++it;
    }

#if defined(FORDIL_DEBUG_DATA)
    qDebug() << "calculated site x longitude min" << longitudeMin << "max" << longitudeMax;
    qDebug() << "calculated site y latitude  min" << latitudeMin << "max" << latitudeMax;
#endif

    double longitudeRange = longitudeMax - longitudeMin;
    double latitudeRange = latitudeMax - latitudeMin;

    // Longitude map area
    double longitudeDegreesPerMeter = dataLongitudeDegreesPerMeter(latitudeMin);

    double longitudeMapMargin = longitudeDegreesPerMeter * 20.0; // Extend calculated area by few meters
    double longitudeMapMin = longitudeMin - longitudeMapMargin;
    double longitudeMapMax = longitudeMax + longitudeMapMargin;
    double longitudeMapRange = longitudeMapMax - longitudeMapMin;

    // Latitude map area
    double latitudeDegreesPerMeter = dataLatitudeDegreesPerMeter();

    double latitudeMapMargin = latitudeDegreesPerMeter * 20.0; // Extend calculated area by few meters
    double latitudeMapMin = latitudeMin - latitudeMapMargin;
    double latitudeMapMax = latitudeMax + latitudeMapMargin;
    double latitudeMapRange = latitudeMapMax -latitudeMapMin;

    // Margin
    m_view.setMargin(0.0);

    // Set site location
    m_site.setLocation(latitudeMapMin, longitudeMapMin,
                       latitudeMapMax, longitudeMapMax,
                       latitudeDegreesPerMeter, longitudeDegreesPerMeter);

    // Set site dimension in meters
    double w = longitudeMapRange / longitudeDegreesPerMeter;
    double h = latitudeMapRange / latitudeDegreesPerMeter;
#if defined(FORDIL_DEBUG_DATA)
    qDebug() << "calculated site w" << w << "h" << h;
#endif
    m_site.setDimension(w, h);
    m_view.setSiteDimensions(w, h);

    // Convert geographic coordinate system to screen coordinate system
    // Bottom-left [ longitude = 0, latitude = 0 ] ->  Top-left [ x = 0, y = 0 ]
    //   direction [ right East   , up           ] -> direction [ right, down  ]
    it = m_spots.begin();
    while (it != m_spots.end())
    {
        double x = (it->second.longitude() - longitudeMapMin) / longitudeDegreesPerMeter;
        double y = (latitudeMapMax - it->second.latitude()) / latitudeDegreesPerMeter;

        it->second.setPosition(x, y);

        ++it;
    }

    // https://www.openstreetmap.org Export area
    qDebug() << "site left   :" << QString::number(longitudeMapMin, 'f', 8);
    qDebug() << "site right  :" << QString::number(longitudeMapMax, 'f', 8);
    qDebug() << "site top    :" << QString::number(latitudeMapMax, 'f', 8);
    qDebug() << "site bottom :" << QString::number(latitudeMapMin, 'f', 8);
}

void Data::loadDiscovery(QXlsx::Document &xlsx)
{
    // Read all data lines
    Table table = xlsxToTable(xlsx, "Discovery", false);

    // Process
    for (size_t i = 0; i < table.rowCount(); i++)
    {
        uint64_t stateId = table.toULongLong("state", i);
        uint64_t stopId = table.toULongLong("stop", i);

        QString treeString = table.text("treeid", i);
        uint64_t treeId = 0;
        if (!treeString.trimmed().isEmpty())
        {
            QString first = treeString.split(',', Qt::SkipEmptyParts).value(0).trimmed();
            bool ok = false;
            treeId = first.toULongLong(&ok);
        }

        QString text = table.text("text", i);
        core().insertTranslation(text, table.text("text cs", i), "cs");
        core().insertTranslation(text, table.text("text fr", i), "fr");

        // Add
        m_discoveryTour.insert(stateId, stopId, treeId, treeString, "header", text);
    }
}

void Data::loadDiscoveryQuiz(QXlsx::Document &xlsx)
{
    // Read all data lines
    Table table = xlsxToTable(xlsx, "Discovery Quiz", false);

    // Process
    for (size_t i = 0; i < table.rowCount(); i++)
    {
        uint64_t questionId = table.toULongLong("question", i);
        uint64_t optionId = table.toULongLong("answer", i);
        QString type = table.text("type", i);
        QString imageName = table.text("image", i);

        QString text = table.text("text", i);
        core().insertTranslation(text, table.text("text cs", i), "cs");
        core().insertTranslation(text, table.text("text fr", i), "fr");

        // Add
        m_discoveryQuiz.insert(questionId, optionId, type, imageName, text);
    }
}

void Data::loadTravailloscopeSpot(QXlsx::Document &xlsx)
{
    // Read all data lines
    Table table = xlsxToTable(xlsx, "Spot", true);

    uint64_t spotIdPrev = 0;
    uint64_t questionIdPrev = 0;
    uint64_t questionAnswerId = 0;

    // Process
    for (size_t i = 0; i < table.rowCount(); i++)
    {
        // Spot ID
        uint64_t spotId = table.toULongLong("spot", i);
        bool newSpot = false;
        if (spotId > 0)
        {
            if (spotId != spotIdPrev)
            {
                spotIdPrev = spotId;
                newSpot = true;

                questionIdPrev = 0;
            }
        }
        else
        {
            spotId = spotIdPrev;
        }

        if (spotId == 0)
        {
            QString msg = "row " + QString::number(i) +
                          ", unknown spot number";
            throw std::runtime_error(msg.toStdString());
        }

        // Coordinates
        double latitude = table.toDouble("wgs lat n", i);
        double longitude = table.toDouble("wgs lon e", i);

        if (latitude > 90.0 || longitude > 180.0)
        {
            latitude = 49.3326 + (randomDouble(1) * 0.001);
            longitude = 16.6926 + (randomDouble(1) * 0.0015);
        }

        // Diameter [m]
        double radius = table.toDouble("diameter [m]", i) * 0.5;

        // Question ID
        uint64_t questionId = table.toULongLong("question", i);
        if (questionId > 0)
        {
            if (questionId != questionIdPrev)
            {
                questionAnswerId = 0;
            }

            questionIdPrev = questionId;
        }
        else
        {
            questionId = questionIdPrev;
        }

        if (questionId == 0)
        {
            QString msg = "row " + QString::number(i) +
                          ", spot " + QString::number(spotId) +
                          ", unknown question number";
            throw std::runtime_error(msg.toStdString());
        }

        // Answer
        QString answer = table.text("answer", i);
        QString answerText;
        QString explanationText;
        bool correct = true;
        bool ok = false;

        questionAnswerId++;

        uint64_t answerId = answer.toULongLong(&ok);
        if (answerId < 1)
        {
            answerId = questionAnswerId;

            correct = (table.text("correct", i).toLower() == "correct");

            answerText = answer;
            core().insertTranslation(answerText, table.text("answer cs", i), "cs");
            core().insertTranslation(answerText, table.text("answer fr", i), "fr");

            explanationText = table.text("pedagogical content", i);
            core().insertTranslation(explanationText, table.text("pedagogical content cs", i), "cs");
            core().insertTranslation(explanationText, table.text("pedagogical content fr", i), "fr");
        }

        // Add new item
#if defined(FORDIL_DEBUG_DATA)
        qDebug() << "row" << i << "spot" << spotId
                 << "questionId" << questionId << "answerId" << answerId;
#endif

        if (newSpot)
        {
            Spot spot(spotId, latitude, longitude, radius);
            m_spots.insert_or_assign(spotId, std::move(spot));

            TravailloscopeSpot quizSpot(spotId);
            m_travailloscopeQuiz.insert(quizSpot);
        }

        TravailloscopeSpotAnswer a(spotId, questionId, answerId, answerText, explanationText, correct);
        m_travailloscopeQuiz.insert(a);
    }

    m_travailloscopeQuiz.shuffle();
    updateSiteDimFromSpots();
}

void Data::loadTravailloscopeQuestion(QXlsx::Document &xlsx)
{
    // Read all data lines
    Table table = xlsxToTable(xlsx, "Question", true);

    // Process
    for (size_t i = 0; i < table.rowCount(); i++)
    {
        uint64_t questionId = table.toULongLong("question", i);
        QString text = table.text("text", i);
        core().insertTranslation(text, table.text("text cs", i), "cs");
        core().insertTranslation(text, table.text("text fr", i), "fr");

        // Add
        m_travailloscopeQuiz.insert(questionId, text);
    }
}

void Data::loadTravailloscopeAnswer(QXlsx::Document &xlsx)
{
    // Read all data lines
    Table table = xlsxToTable(xlsx, "Answer", true);

    // Process
    for (size_t i = 0; i < table.rowCount(); i++)
    {
        uint64_t questionId = table.toULongLong("question #", i);
        uint64_t answerId = table.toULongLong("answer #", i);

        QString answerText = table.text("answer", i);
        core().insertTranslation(answerText, table.text("answer cs", i), "cs");
        core().insertTranslation(answerText, table.text("answer fr", i), "fr");

        QString explanationText = table.text("explanation", i);
        core().insertTranslation(explanationText, table.text("explanation cs", i), "cs");
        core().insertTranslation(explanationText, table.text("explanation fr", i), "fr");

        // Add new answer
        TravailloscopeAnswer a(questionId, answerId, answerText, explanationText);
        m_travailloscopeQuiz.insert(a);
    }
}

static void dataInspectDataFile(const QString &filePath,
                                QXlsx::Document &xlsx,
                                QString &system,
                                QString &siteName,
                                QString &error)
{
    if (!xlsx.selectSheet("Site"))
    {
        error = QCoreApplication::translate("Backend", "Sheet 'Site' not found.");
        return;
    }

    QVariant siteCell = xlsx.read(2, 1);
    if (!siteCell.isValid())
    {
        error = QCoreApplication::translate("Backend", "'Site.Site' name is not valid");
        return;
    }

    siteName = siteCell.toString().trimmed();
    if (siteName.isEmpty())
    {
        error = QCoreApplication::translate("Backend", "'Site.Site' name is empty");
        return;
    }

    if (xlsx.selectSheet("Tree"))
    {
        system = "Marteloscope";
        return;
    }

    if (xlsx.selectSheet("Spot"))
    {
        system = "Travailloscope";
        return;
    }

    error = QCoreApplication::translate("Backend", "Unknown site system. Expected Marteloscope or Travailloscope.");
}

void Data::inspectDataFile(const QString &filePath,
                           QString &system,
                           QString &siteName,
                           QString &error)
{
    QXlsx::Document xlsx(filePath);
    dataInspectDataFile(filePath, xlsx, system, siteName, error);
}

QString Data::validateDataFile(const QString &filePath,
                               const QString &system,
                               bool discovery)
{
    try
    {
        QXlsx::Document xlsx(filePath);

        QString systemInFile;
        QString siteName;
        QString error;
        dataInspectDataFile(filePath, xlsx, systemInFile, siteName, error);

        if (siteName.isEmpty())
        {
            qDebug() << "Cell 'Site' name is empty" << filePath;
            return QString();
        }

        if (system == "Marteloscope")
        {
            if (systemInFile != "Marteloscope")
            {
                // Ignore this site, it is not a Marteloscope
                return QString();
            }

            // Check if discovery is available for this site
            if (discovery)
            {
                if (!xlsx.selectSheet("Discovery"))
                {
                    // Ignore this site, discovery is not available
                    return QString();
                }

                QXlsx::CellRange range = xlsx.dimension();
                if (range.lastRow() - range.firstRow() < 2)
                {
                    // Ignore this site, discovery is not available
                    return QString();
                }
            }

            // The site format seems okay
            return siteName;
        }
        else if (system == "Travailloscope")
        {
            if (systemInFile != "Travailloscope")
            {
                // Ignore this site, it is not a Travailloscope
                return QString();
            }

            // The site format seems okay
            return siteName;
        }
    }
    catch (...)
    {
        qWarning() << "Failed to read file" << filePath;
    }

    return QString();
}

QString Data::bundledSitePath(const QString &filePath,
                              const QString &siteName,
                              const QString &system)
{
    QString path = filePath;

    // Use initial site data bundled with the application if necessary.
    if (path.isEmpty())
    {
        if (system == "Marteloscope")
        {
#if defined(FORDIL_BUNDLE_SITE)
            if (siteName == "Mormal FR") path = ":/data/Marteloscope_Mormal_FR.xlsx";
            if (siteName == "Lauzelle BE") path = ":/data/Marteloscope_Lauzelle_BE.xlsx";
            if (siteName == "Pokojna CZ") path = ":/data/Marteloscope_Pokojna_CZ.xlsx";
#endif
        }
        else if (system == "Travailloscope")
        {
#if defined(FORDIL_BUNDLE_SITE)
            if (siteName == "Mormal FR") path = ":/data/Travailloscope_Mormal_FR.xlsx";
            if (siteName == "Lauzelle BE") path = ":/data/Travailloscope_Lauzelle_BE.xlsx";
            if (siteName == "Pokojna CZ") path = ":/data/Travailloscope_Pokojna_CZ.xlsx";
#endif
        }
        else
        {
            throw std::runtime_error("Unknown system \"" + system.toStdString() + "\"");
        }

        qDebug() << "updated path" << path;
    }

    return path;
}

void Data::loadExercise(const QString &filePath,
                        const QString &siteName,
                        const QString &system,
                        bool discovery)
{
    qDebug() << "load exercise" << siteName
             << "system" << system
             << "discovery" << discovery
             << "from" << filePath;

    clear();

    QString path = bundledSitePath(filePath, siteName, system);

    QXlsx::Document xlsx(path);

    loadSite(xlsx, siteName, system);

    if (system == "Marteloscope")
    {
        loadIntroduction(xlsx);
        loadSpecies(xlsx);
        loadEcologicalValues(xlsx);
        loadTrees(xlsx);
        loadTreeLogs(xlsx);
        loadTypology(xlsx);
        loadExperts(xlsx);
        loadHotspots(xlsx);
        loadDiscoveryQuiz(xlsx);
        loadDiscovery(xlsx);
    }
    else if (system == "Travailloscope")
    {
        loadIntroduction(xlsx);
        loadTravailloscopeQuestion(xlsx);
        loadTravailloscopeAnswer(xlsx);
        loadTravailloscopeSpot(xlsx);
    }
    else
    {
        throw std::runtime_error("Unknown system \"" + system.toStdString() + "\"");
    }

    setExerciseFinished(false);

    m_discoveryPart = discovery;
}

void Data::clear()
{
    core().clear();

    m_view = View();
    m_site = Site();
    m_typology = Typology();

    m_species.clear();
    m_speciesFullName.clear();
    m_speciesSite.clear();

    m_ecologicalValues.clear();

    m_trees.clear();
    m_treeIdsSortedByDbh.clear();

    m_hotspotList.clear();
    m_spots.clear();
    m_travailloscopeQuiz.clear();
    m_introduction.clear();
    // m_text.clear();

    m_discoveryQuiz.clear();
    m_discoveryTour.clear();
    m_discoveryState = 1;
}

void Data::setExerciseFinished(bool value)
{
    m_exerciseFinished = value;

    if (isExerciseFinished())
    {
        m_endTime = QDateTime::currentDateTime();
    }
    else
    {
        m_startTime = QDateTime::currentDateTime();
        m_endTime = m_startTime; 
    }
}

QString Data::elapsedTime() const
{
    QDateTime last;

    if (isExerciseFinished())
    {
        last = m_endTime;
    }
    else
    {
        last = QDateTime::currentDateTime();
    }
    
    qint64 elapsedSeconds = m_startTime.secsTo(last);

    int hours = static_cast<int>(elapsedSeconds / 3600);
    int minutes = static_cast<int>((elapsedSeconds % 3600) / 60);
    int seconds = static_cast<int>(elapsedSeconds % 60);

    QString elapsedFormatted = QString("%1:%2:%3")
                                   .arg(hours, 2, 10, QChar('0'))
                                   .arg(minutes, 2, 10, QChar('0'))
                                   .arg(seconds, 2, 10, QChar('0'));
    
    return elapsedFormatted;
}

QString Data::introductionSiteHeader() const
{
    return loadedExerciseName();
}

QString Data::introductionSiteBody() const
{
    QString str = core().translate(m_introduction);
    formatTextImageSize(str);
    return str;
}

// Selection
bool Data::setTreePreselected(SelectedTree &selectedTree, uint64_t treeId)
{
    for (auto &[key, tree] : m_trees)
    {
        tree.setSelectionStatus(SelectionStatus::None);
    }

    auto it = m_trees.find(treeId);
    if (it != m_trees.end())
    {
        it->second.setSelectionStatus(SelectionStatus::Highlighted);
        const auto &species = m_species.at(it->second.speciesId());
        selectedTree.pick(it->second, species);

        return true;
    }

    return false;
}

bool Data::setSpotPreselected(SelectedSpot &selectedSpot, uint64_t spotId)
{
    for (auto &[key, spot] : m_spots)
    {
        spot.setSelectionStatus(SelectionStatus::None);
    }

    auto it = m_spots.find(spotId);
    if (it != m_spots.end())
    {
        it->second.setSelectionStatus(SelectionStatus::Highlighted);
        selectedSpot.pick(it->second);

        return true;
    }

    return false;
}

void Data::calculateTypologyLocal()
{
    m_typology.calculateLocal(m_trees);
}

void Data::calculateTypologyGlobal()
{
    m_typology.calculateGlobal();
}

void Data::calculateOverallAssessment()
{
    m_typology.calculateOverallAssessment();
}

QString Data::travailloscopeQuestionHeader(uint64_t questionId) const
{
    return m_travailloscopeQuiz.questionHeader(questionId);
}

void Data::clearTravailloscopeAnswers(uint64_t spotId)
{
    m_travailloscopeQuiz.clearAnswers(spotId);
}

void Data::setTravailloscopeAnswer(uint64_t spotId, uint64_t question, const std::set<uint64_t> &answer)
{
    m_travailloscopeQuiz.setAnswer(spotId, question, answer);
}

bool Data::isTravailloscopeSpotFinished(uint64_t spotId) const
{
    return m_travailloscopeQuiz.isFinished(spotId);
}

int Data::travailloscopeScore(uint64_t spotId, uint64_t question) const
{
    return m_travailloscopeQuiz.calculateScore(spotId, question);
}

QString Data::travailloscopePedagogicalHeader(uint64_t spotId) const
{
    return QCoreApplication::translate("Backend", "Answers for spot n°") +
           QString::number(spotId);
}

QString Data::travailloscopePedagogicalContents(uint64_t spotId, uint64_t question) const
{
    QString str = m_travailloscopeQuiz.pedagogicalContents(spotId, question);
    formatTextImageSize(str);
    return str;
}

void Data::travailloscopeCalculateScore()
{
    m_travailloscopeQuiz.calculateScore();
}

QString Data::travailloscopeScoreText() const
{
    return m_travailloscopeQuiz.totalScoreText();
}

QString Data::travailloscopeScoreColor() const
{
    return m_travailloscopeQuiz.totalScoreColor();
}

QVariantList Data::travailloscopeScoreModelData() const
{
    return m_travailloscopeQuiz.scoreModelData();
}

// Discovery
void Data::discoveryResetQuiz()
{
    m_discoveryStep = 1;
}

int Data::discoveryQuestion()
{
    return m_discoveryStep;
}

bool Data::discoveryHasQuestions()
{
    return m_discoveryQuiz.size() > 0;
}

bool Data::discoveryHasNextQuestion()
{
    return m_discoveryStep < static_cast<int>(m_discoveryQuiz.size());
}

void Data::discoveryNextQuestion()
{
    m_discoveryStep++;
}

QString Data::discoveryQuestionHeader(uint64_t questionId)
{
    return m_discoveryQuiz.questionHeader(questionId);
}

QVariantList Data::discoveryCheckBoxItems(uint64_t questionId)
{
    QVariantList list;

    if (!m_discoveryQuiz.hasQuestion(questionId))
    {
        return list;
    }

    const DiscoveryQuestion &question = m_discoveryQuiz.question(questionId);

    for (const auto &option : question.m_options)
    {
        int id = static_cast<int>(option.m_optionId);

        QString text = QString::number(id) + ". " + core().translate(option.m_text);

        QString image;
        if (!option.m_imageName.isEmpty())
        {
            image = "<img src='qrc:/images/" + option.m_imageName + "' width='1'/>";
            formatTextImageSize(image);
        }

        list << QVariantMap{{"id", id},
                            {"text", text},
                            {"image", image}};
    }

    return list;
}

QString Data::discoveryQuestionText(uint64_t questionId)
{
    return m_discoveryQuiz.questionText(questionId);
}

QString Data::discoveryAnswerHeader(uint64_t questionId)
{
    return m_discoveryQuiz.answerHeader(questionId);
}

QString Data::discoveryAnswerText(uint64_t questionId)
{
    QString str = m_discoveryQuiz.answerText(questionId);
    formatTextImageSize(str);
    return str;
}

// Discovery tour
int Data::discoveryState()
{
    return m_discoveryState;
}

bool Data::discoveryHasNextState()
{
    return m_discoveryState < static_cast<int>(m_discoveryTour.size());
}

void Data::discoveryNextState()
{
    m_discoveryState++;
}

int Data::discoveryStop()
{
    return m_discoveryTour.stop(m_discoveryState);
}

uint64_t Data::discoveryTreeId()
{
    return m_discoveryTour.treeId(m_discoveryState);
}

QString Data::discoveryStopMessage()
{
    return m_discoveryTour.message(m_discoveryState);
}

QString Data::discoveryHeader()
{
    return m_discoveryTour.header(m_discoveryState);
}

QString Data::discoveryText()
{
    QString str = m_discoveryTour.text(m_discoveryState);
    formatTextImageSize(str);
    return str;
}

// Format
void Data::italicizeWords(QString& html, const QSet<QString>& words) const
{
    QString pattern = "\\b(" + QStringList(words.begin(), words.end()).join("|") + ")\\b";
    QRegularExpression re(pattern);

    html.replace(re, "<i>\\1</i>");
}

void Data::italicizeWords(QString& html) const
{
    QSet<QString> words = {
        "Abies alba",
        "Acer sp.",
        "Acer pseudoplatanus",
        "Alnus sp.",
        "Betula pendula",
        "Betula sp.",
        "Carpinus betulus",
        "Fagus sylvatica",
        "Fraxinus excelsior",
        "Fraxinus Excelsior",
        "Larix decidua",
        "Larix sp.",
        "Picea abies",
        "Picea sp.",
        "Pinus sylvestris",
        "Pinus sp.",
        "Populus sp.",
        "Prunus avium",
        "Prunus Avium",
        "Prunus serotina",
        "Prunus Serotina",
        "Prunus sp.",
        "Pteridium aquilinum",
        "Rubus sp.",
        "Quercus petraea",
        "Quercus robur",
        "Quercus sp."
    };

    italicizeWords(html, words);
}

void Data::setupWindow(int screenWidth, int screenHeight)
{
    if (screenWidth < 100 || screenHeight < 100)
    {
        return;
    }

    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
}

void Data::formatTextImageSize(QString &str,
                               int maxWidth,
                               int autoWidth) const
{
    italicizeWords(str);

    if (str.indexOf("<img") < 0)
    {
        return;
    }

    static const QRegularExpression imgRe(
        R"(<img\b[^>]*?/?>)",
        QRegularExpression::CaseInsensitiveOption);

    static const QRegularExpression srcRe(
        R"(\bsrc\s*=\s*(['"])(.*?)\1)",
        QRegularExpression::CaseInsensitiveOption);

    static const QRegularExpression widthRe(
        R"(\bwidth\s*=\s*(['"])(.*?)\1)",
        QRegularExpression::CaseInsensitiveOption);

    QString out = str;
    int offset = 0;

    if (maxWidth < 1)
    {
        maxWidth = m_screenWidth;
    }

    auto it = imgRe.globalMatch(out);
    while (it.hasNext())
    {
        const auto m = it.next();
        const int start = m.capturedStart() + offset;
        const int len   = m.capturedLength();

        QString tag = out.mid(start, len);

        // ---- extract src ----
        QRegularExpressionMatch srcMatch = srcRe.match(tag);
        if (!srcMatch.hasMatch())
        {
            continue;   // skip this <img>
        }

        // ---- width ----
        int marginWidth = 50;
        int newWidth = 0;
        int newHeight = 1;

        newWidth = maxWidth - marginWidth;

        if (newWidth < 1 || newHeight < 1)
        {
            continue;
        }

        // ---- insert width ----
        QRegularExpressionMatch widthMatch = widthRe.match(tag);
        if (widthMatch.hasMatch())
        {
            const QString widthStr = widthMatch.captured(2); // value inside quotes
            bool ok = false;
            int width = widthStr.toInt(&ok);

            if (!ok)
            {
                continue; // keep tag unchanged
            }

            if (width != 1 && width < newWidth)
            {
                continue; // keep tag unchanged
            }

            if (width == 1)
            {
                width = autoWidth;
            }

            if (width > newWidth)
            {
                width = newWidth;
            }

            // replace
            tag.replace(widthRe, QString("width='%1'").arg(width));
        }
        else
        {
            // no width -> insert
            int insertPos = tag.lastIndexOf("/>");

            if (insertPos < 0)
            {
                insertPos = tag.lastIndexOf(">");
            }

            if (insertPos < 0)
            {
                continue;
            }

            tag.insert(insertPos, QString(" width='%1'").arg(newWidth));
        }

        out.replace(start, len, tag);
        offset += tag.size() - len;
    }

    str = out;
}

QString Data::text(const QString &identifier) const
{
    if (m_text.count(identifier) < 1)
    {
        return QString("Resource identifer not found");
    }

    QString str = m_text.at(identifier);

    formatTextImageSize(str);

    return str;
}

void Data::loadText()
{
    m_text.clear();

    // App
    constexpr char trApp1a[] = QT_TRANSLATE_NOOP("Backend", "App: Introduction Header");
    m_text[trApp1a] = QCoreApplication::translate("Backend", trApp1a);
    constexpr char trApp1b[] = QT_TRANSLATE_NOOP("Backend", "App: Introduction");
    m_text[trApp1b] = QCoreApplication::translate("Backend", trApp1b);

    // Discovery
    constexpr char trDiscovery1a[] = QT_TRANSLATE_NOOP("Backend", "Discovery: End Message Header");
    m_text[trDiscovery1a] = QCoreApplication::translate("Backend", trDiscovery1a);
    constexpr char trDiscovery1b[] = QT_TRANSLATE_NOOP("Backend", "Discovery: End Message");
    m_text[trDiscovery1b] = QCoreApplication::translate("Backend", trDiscovery1b);

    // Selection
    constexpr char trMarteloscope1a[] = QT_TRANSLATE_NOOP("Backend", "Marteloscope: Competition Header");
    m_text[trMarteloscope1a] = QCoreApplication::translate("Backend", trMarteloscope1a);
    constexpr char trMarteloscope1b[] = QT_TRANSLATE_NOOP("Backend", "Marteloscope: Competition Explanation");
    m_text[trMarteloscope1b] = QCoreApplication::translate("Backend", trMarteloscope1b);

    constexpr char trMarteloscope2a[] = QT_TRANSLATE_NOOP("Backend", "Marteloscope: Maturity Header");
    m_text[trMarteloscope2a] = QCoreApplication::translate("Backend", trMarteloscope2a);
    constexpr char trMarteloscope2b[] = QT_TRANSLATE_NOOP("Backend", "Marteloscope: Maturity Explanation");
    m_text[trMarteloscope2b] = QCoreApplication::translate("Backend", trMarteloscope2b);

    constexpr char trMarteloscope3a[] = QT_TRANSLATE_NOOP("Backend", "Marteloscope: Promote Regeneration Header");
    m_text[trMarteloscope3a] = QCoreApplication::translate("Backend", trMarteloscope3a);
    constexpr char trMarteloscope3b[] = QT_TRANSLATE_NOOP("Backend", "Marteloscope: Promote Regeneration Explanation");
    m_text[trMarteloscope3b] = QCoreApplication::translate("Backend", trMarteloscope3b);

    constexpr char trMarteloscope4a[] = QT_TRANSLATE_NOOP("Backend", "Marteloscope: Promote Rare Species Header");
    m_text[trMarteloscope4a] = QCoreApplication::translate("Backend", trMarteloscope4a);
    constexpr char trMarteloscope4b[] = QT_TRANSLATE_NOOP("Backend", "Marteloscope: Promote Rare Species Explanation");
    m_text[trMarteloscope4b] = QCoreApplication::translate("Backend", trMarteloscope4b);

    constexpr char trMarteloscope5a[] = QT_TRANSLATE_NOOP("Backend", "Marteloscope: Sanitary Header");
    m_text[trMarteloscope5a] = QCoreApplication::translate("Backend", trMarteloscope5a);
    constexpr char trMarteloscope5b[] = QT_TRANSLATE_NOOP("Backend", "Marteloscope: Sanitary Explanation");
    m_text[trMarteloscope5b] = QCoreApplication::translate("Backend", trMarteloscope5b);

    constexpr char trMarteloscope6a[] = QT_TRANSLATE_NOOP("Backend", "Marteloscope: Technical Header");
    m_text[trMarteloscope6a] = QCoreApplication::translate("Backend", trMarteloscope6a);
    constexpr char trMarteloscope6b[] = QT_TRANSLATE_NOOP("Backend", "Marteloscope: Technical Explanation");
    m_text[trMarteloscope6b] = QCoreApplication::translate("Backend", trMarteloscope6b);

    constexpr char trMarteloscope7a[] = QT_TRANSLATE_NOOP("Backend", "Marteloscope: Bad Quality Header");
    m_text[trMarteloscope7a] = QCoreApplication::translate("Backend", trMarteloscope7a);
    constexpr char trMarteloscope7b[] = QT_TRANSLATE_NOOP("Backend", "Marteloscope: Bad Quality Explanation");
    m_text[trMarteloscope7b] = QCoreApplication::translate("Backend", trMarteloscope7b);

    // Results
    constexpr char trResults0a[] = QT_TRANSLATE_NOOP("Backend", "Results: Map");
    m_text[trResults0a] = QCoreApplication::translate("Backend", trResults0a);

    constexpr char trResults0b[] = QT_TRANSLATE_NOOP("Backend", "Results: Marteloscope Results");
    m_text[trResults0b] = QCoreApplication::translate("Backend", trResults0b);
    constexpr char trResults0c[] = QT_TRANSLATE_NOOP("Backend", "Results: Global Results Introduction Header");
    m_text[trResults0c] = QCoreApplication::translate("Backend", trResults0c);
    constexpr char trResults0d[] = QT_TRANSLATE_NOOP("Backend", "Results: Global Results Introduction");
    m_text[trResults0d] = QCoreApplication::translate("Backend", trResults0d);

    constexpr char trResults1a[] = QT_TRANSLATE_NOOP("Backend", "Results: Tree Distribution Header");
    m_text[trResults1a] = QCoreApplication::translate("Backend", trResults1a);
    constexpr char trResults1b[] = QT_TRANSLATE_NOOP("Backend", "Results: Tree Distribution Explanation Header");
    m_text[trResults1b] = QCoreApplication::translate("Backend", trResults1b);
    constexpr char trResults1c[] = QT_TRANSLATE_NOOP("Backend", "Results: Tree Distribution Explanation");
    m_text[trResults1c] = QCoreApplication::translate("Backend", trResults1c);

    constexpr char trResults2a[] = QT_TRANSLATE_NOOP("Backend", "Results: Tree Statistics Header");
    m_text[trResults2a] = QCoreApplication::translate("Backend", trResults2a);
    constexpr char trResults2b[] = QT_TRANSLATE_NOOP("Backend", "Results: Tree Statistics Explanation Header");
    m_text[trResults2b] = QCoreApplication::translate("Backend", trResults2b);
    constexpr char trResults2c[] = QT_TRANSLATE_NOOP("Backend", "Results: Tree Statistics Explanation");
    m_text[trResults2c] = QCoreApplication::translate("Backend", trResults2c);

    constexpr char trResults3a[] = QT_TRANSLATE_NOOP("Backend", "Results: K Factor Header");
    m_text[trResults3a] = QCoreApplication::translate("Backend", trResults3a);
    constexpr char trResults3b[] = QT_TRANSLATE_NOOP("Backend", "Results: K Factor Explanation Header");
    m_text[trResults3b] = QCoreApplication::translate("Backend", trResults3b);
    constexpr char trResults3c[] = QT_TRANSLATE_NOOP("Backend", "Results: K Factor Explanation");
    m_text[trResults3c] = QCoreApplication::translate("Backend", trResults3c);

    constexpr char trResults4a[] = QT_TRANSLATE_NOOP("Backend", "Results: Gini Index Header");
    m_text[trResults4a] = QCoreApplication::translate("Backend", trResults4a);
    constexpr char trResults4b[] = QT_TRANSLATE_NOOP("Backend", "Results: Gini Index Explanation Header");
    m_text[trResults4b] = QCoreApplication::translate("Backend", trResults4b);
    constexpr char trResults4c[] = QT_TRANSLATE_NOOP("Backend", "Results: Gini Index Explanation");
    m_text[trResults4c] = QCoreApplication::translate("Backend", trResults4c);

    constexpr char trResults5a[] = QT_TRANSLATE_NOOP("Backend", "Results: CCF Model Curves Header");
    m_text[trResults5a] = QCoreApplication::translate("Backend", trResults5a);
    constexpr char trResults5b[] = QT_TRANSLATE_NOOP("Backend", "Results: CCF Model Curves Explanation Header");
    m_text[trResults5b] = QCoreApplication::translate("Backend", trResults5b);
    constexpr char trResults5c[] = QT_TRANSLATE_NOOP("Backend", "Results: CCF Model Curves Explanation");
    m_text[trResults5c] = QCoreApplication::translate("Backend", trResults5c);

    constexpr char trResults6a[] = QT_TRANSLATE_NOOP("Backend", "Results: Quality - before Header");
    m_text[trResults6a] = QCoreApplication::translate("Backend", trResults6a);
    constexpr char trResults6b[] = QT_TRANSLATE_NOOP("Backend", "Results: Quality - cut Header");
    m_text[trResults6b] = QCoreApplication::translate("Backend", trResults6b);
    constexpr char trResults6c[] = QT_TRANSLATE_NOOP("Backend", "Results: Quality - after Header");
    m_text[trResults6c] = QCoreApplication::translate("Backend", trResults6c);
    constexpr char trResults6d[] = QT_TRANSLATE_NOOP("Backend", "Results: Quality Evolution Header");
    m_text[trResults6d] = QCoreApplication::translate("Backend", trResults6d);
    constexpr char trResults6e[] = QT_TRANSLATE_NOOP("Backend", "Results: Quality Explanation Header");
    m_text[trResults6e] = QCoreApplication::translate("Backend", trResults6e);
    constexpr char trResults6f[] = QT_TRANSLATE_NOOP("Backend", "Results: Quality Explanation");
    m_text[trResults6f] = QCoreApplication::translate("Backend", trResults6f);

    constexpr char trResults7a[] = QT_TRANSLATE_NOOP("Backend", "Results: Composition - before Header");
    m_text[trResults7a] = QCoreApplication::translate("Backend", trResults7a);
    constexpr char trResults7b[] = QT_TRANSLATE_NOOP("Backend", "Results: Composition - cut Header");
    m_text[trResults7b] = QCoreApplication::translate("Backend", trResults7b);
    constexpr char trResults7c[] = QT_TRANSLATE_NOOP("Backend", "Results: Composition - after Header");
    m_text[trResults7c] = QCoreApplication::translate("Backend", trResults7c);
    constexpr char trResults7d[] = QT_TRANSLATE_NOOP("Backend", "Results: Composition Explanation Header");
    m_text[trResults7d] = QCoreApplication::translate("Backend", trResults7d);
    constexpr char trResults7e[] = QT_TRANSLATE_NOOP("Backend", "Results: Composition Explanation");
    m_text[trResults7e] = QCoreApplication::translate("Backend", trResults7e);

    constexpr char trResults8a[] = QT_TRANSLATE_NOOP("Backend", "Results: Shannon Index Header");
    m_text[trResults8a] = QCoreApplication::translate("Backend", trResults8a);
    constexpr char trResults8b[] = QT_TRANSLATE_NOOP("Backend", "Results: Shannon Index Explanation Header");
    m_text[trResults8b] = QCoreApplication::translate("Backend", trResults8b);
    constexpr char trResults8c[] = QT_TRANSLATE_NOOP("Backend", "Results: Shannon Index Explanation");
    m_text[trResults8c] = QCoreApplication::translate("Backend", trResults8c);

    constexpr char trResults9a[] = QT_TRANSLATE_NOOP("Backend", "Results: Choices Assessment Header");
    m_text[trResults9a] = QCoreApplication::translate("Backend", trResults9a);
    constexpr char trResults9b[] = QT_TRANSLATE_NOOP("Backend", "Results: Choices Assessment Explanation Header");
    m_text[trResults9b] = QCoreApplication::translate("Backend", trResults9b);
    constexpr char trResults9c[] = QT_TRANSLATE_NOOP("Backend", "Results: Choices Assessment Explanation");
    m_text[trResults9c] = QCoreApplication::translate("Backend", trResults9c);

    constexpr char trResults10a[] = QT_TRANSLATE_NOOP("Backend", "Results: Dead and Biological Trees Header");
    m_text[trResults10a] = QCoreApplication::translate("Backend", trResults10a);
    constexpr char trResults10b[] = QT_TRANSLATE_NOOP("Backend", "Results: Dead and Biological Trees Explanation Header");
    m_text[trResults10b] = QCoreApplication::translate("Backend", trResults10b);
    constexpr char trResults10c[] = QT_TRANSLATE_NOOP("Backend", "Results: Dead and Biological Trees Explanation");
    m_text[trResults10c] = QCoreApplication::translate("Backend", trResults10c);

    constexpr char trResults11a[] = QT_TRANSLATE_NOOP("Backend", "Results: Dendromicrohabitats (DMH) Header");
    m_text[trResults11a] = QCoreApplication::translate("Backend", trResults11a);
    constexpr char trResults11b[] = QT_TRANSLATE_NOOP("Backend", "Results: Dendromicrohabitats (DMH) Explanation Header");
    m_text[trResults11b] = QCoreApplication::translate("Backend", trResults11b);
    constexpr char trResults11c[] = QT_TRANSLATE_NOOP("Backend", "Results: Dendromicrohabitats (DMH) Explanation");
    m_text[trResults11c] = QCoreApplication::translate("Backend", trResults11c);

    constexpr char trResults12a[] = QT_TRANSLATE_NOOP("Backend", "Results: Prosilva Header");
    m_text[trResults12a] = QCoreApplication::translate("Backend", trResults12a);
    constexpr char trResults12b[] = QT_TRANSLATE_NOOP("Backend", "Results: Prosilva Explanation Header");
    m_text[trResults12b] = QCoreApplication::translate("Backend", trResults12b);
    constexpr char trResults12c[] = QT_TRANSLATE_NOOP("Backend", "Results: Prosilva Explanation");
    m_text[trResults12c] = QCoreApplication::translate("Backend", trResults12c);

    constexpr char trResults13a[] = QT_TRANSLATE_NOOP("Backend", "Results: Economic Values Header");
    m_text[trResults13a] = QCoreApplication::translate("Backend", trResults13a);
    constexpr char trResults13b[] = QT_TRANSLATE_NOOP("Backend", "Results: Economic Values Explanation Header");
    m_text[trResults13b] = QCoreApplication::translate("Backend", trResults13b);
    constexpr char trResults13c[] = QT_TRANSLATE_NOOP("Backend", "Results: Economic Values Explanation");
    m_text[trResults13c] = QCoreApplication::translate("Backend", trResults13c);

    constexpr char trResults14a[] = QT_TRANSLATE_NOOP("Backend", "Results: Economic Values (Expert) Header");
    m_text[trResults14a] = QCoreApplication::translate("Backend", trResults14a);

    constexpr char trCurve01a[] = QT_TRANSLATE_NOOP("Backend", "mixed forest - type A (Meyer)");
    m_text[trCurve01a] = QCoreApplication::translate("Backend", trCurve01a);
    constexpr char trCurve02a[] = QT_TRANSLATE_NOOP("Backend", "mixed forest - type B (Meyer)");
    m_text[trCurve02a] = QCoreApplication::translate("Backend", trCurve02a);
    constexpr char trCurve03a[] = QT_TRANSLATE_NOOP("Backend", "mixed forest - type C (Meyer)");
    m_text[trCurve03a] = QCoreApplication::translate("Backend", trCurve03a);
    constexpr char trCurve04a[] = QT_TRANSLATE_NOOP("Backend", "mixed forest - type D (Meyer)");
    m_text[trCurve04a] = QCoreApplication::translate("Backend", trCurve04a);
    constexpr char trCurve05a[] = QT_TRANSLATE_NOOP("Backend", "mixed forest - type E (Meyer)");
    m_text[trCurve05a] = QCoreApplication::translate("Backend", trCurve05a);
    constexpr char trCurve06a[] = QT_TRANSLATE_NOOP("Backend", "beech forest (Biehl)");
    m_text[trCurve06a] = QCoreApplication::translate("Backend", trCurve06a);

    // Advanced Results
    constexpr char trResults15a[] = QT_TRANSLATE_NOOP("Backend", "Results: Marteloscope Advanced Results");
    m_text[trResults15a] = QCoreApplication::translate("Backend", trResults15a);
    constexpr char trResults15b[] = QT_TRANSLATE_NOOP("Backend", "Results: Advanced Results Introduction Header");
    m_text[trResults15b] = QCoreApplication::translate("Backend", trResults15b);
    constexpr char trResults15c[] = QT_TRANSLATE_NOOP("Backend", "Results: Advanced Results Introduction");
    m_text[trResults15c] = QCoreApplication::translate("Backend", trResults15c);

    constexpr char trResults16a[] = QT_TRANSLATE_NOOP("Backend", "Results: Local Typology Header");
    m_text[trResults16a] = QCoreApplication::translate("Backend", trResults16a);
    constexpr char trResults16b[] = QT_TRANSLATE_NOOP("Backend", "Results: Local Typology Before Header");
    m_text[trResults16b] = QCoreApplication::translate("Backend", trResults16b);
    constexpr char trResults16c[] = QT_TRANSLATE_NOOP("Backend", "Results: Local Typology After Header");
    m_text[trResults16c] = QCoreApplication::translate("Backend", trResults16c);
    constexpr char trResults16d[] = QT_TRANSLATE_NOOP("Backend", "Results: Local Typology Explanation Header");
    m_text[trResults16d] = QCoreApplication::translate("Backend", trResults16d);
    constexpr char trResults16e[] = QT_TRANSLATE_NOOP("Backend", "Results: Local Typology Explanation");
    m_text[trResults16e] = QCoreApplication::translate("Backend", trResults16e);

    constexpr char trResults17a[] = QT_TRANSLATE_NOOP("Backend", "Results: Global Typology Header");
    m_text[trResults17a] = QCoreApplication::translate("Backend", trResults17a);
    constexpr char trResults17b[] = QT_TRANSLATE_NOOP("Backend", "Results: Global Typology Explanation Header");
    m_text[trResults17b] = QCoreApplication::translate("Backend", trResults17b);
    constexpr char trResults17c[] = QT_TRANSLATE_NOOP("Backend", "Results: Global Typology Explanation");
    m_text[trResults17c] = QCoreApplication::translate("Backend", trResults17c);

    constexpr char trResults18a[] = QT_TRANSLATE_NOOP("Backend", "Results: Overall CCF assessment Header");
    m_text[trResults18a] = QCoreApplication::translate("Backend", trResults18a);
    constexpr char trResults18b[] = QT_TRANSLATE_NOOP("Backend", "Results: Overall CCF assessment Explanation Header");
    m_text[trResults18b] = QCoreApplication::translate("Backend", trResults18b);
    constexpr char trResults18c[] = QT_TRANSLATE_NOOP("Backend", "Results: Overall CCF assessment Explanation");
    m_text[trResults18c] = QCoreApplication::translate("Backend", trResults18c);

    constexpr char trResults40a[] = QT_TRANSLATE_NOOP("Backend", "Results: Marteloscope Hotspots Introduction Header");
    m_text[trResults40a] = QCoreApplication::translate("Backend", trResults40a);
    constexpr char trResults40b[] = QT_TRANSLATE_NOOP("Backend", "Results: Marteloscope Hotspots Introduction");
    m_text[trResults40b] = QCoreApplication::translate("Backend", trResults40b);

    constexpr char trResults41a[] = QT_TRANSLATE_NOOP("Backend", "Results: Travailloscope Valuation Introduction Header");
    m_text[trResults41a] = QCoreApplication::translate("Backend", trResults41a);
    constexpr char trResults41b[] = QT_TRANSLATE_NOOP("Backend", "Results: Travailloscope Valuation Introduction");
    m_text[trResults41b] = QCoreApplication::translate("Backend", trResults41b);

    constexpr char trResults50a[] = QT_TRANSLATE_NOOP("Backend", "Results: Text: Expert");
    m_text[trResults50a] = QCoreApplication::translate("Backend", trResults50a);
    constexpr char trResults50b[] = QT_TRANSLATE_NOOP("Backend", "Results: Text: +10 years");
    m_text[trResults50b] = QCoreApplication::translate("Backend", trResults50b);

    // About
    constexpr char trAbout1[] = QT_TRANSLATE_NOOP("Backend", "About: license");
    m_text[trAbout1] = QCoreApplication::translate("Backend", trAbout1);

    constexpr char trAbout2[] = QT_TRANSLATE_NOOP("Backend", "About: 3rd party notice");
    m_text[trAbout2] = QCoreApplication::translate("Backend", trAbout2);

    // Gui
    constexpr char trGui01[] = QT_TRANSLATE_NOOP("Backend", "Export to PDF");
    m_text[trGui01] = QCoreApplication::translate("Backend", trGui01);

    constexpr char trGui02[] = QT_TRANSLATE_NOOP("Backend", "Save File");
    m_text[trGui02] = QCoreApplication::translate("Backend", trGui02);

    constexpr char trGui03[] = QT_TRANSLATE_NOOP("Backend", "Download");
    m_text[trGui03] = QCoreApplication::translate("Backend", trGui03);

    constexpr char trGui04[] = QT_TRANSLATE_NOOP("Backend", "All your results in PDF");
    m_text[trGui04] = QCoreApplication::translate("Backend", trGui04);

    constexpr char trGui05a[] = QT_TRANSLATE_NOOP("Backend", "Import");
    m_text[trGui05a] = QCoreApplication::translate("Backend", trGui05a);

    constexpr char trGui05b[] = QT_TRANSLATE_NOOP("Backend", "Excel Form");
    m_text[trGui05b] = QCoreApplication::translate("Backend", trGui05b);

    constexpr char trGui05c[] = QT_TRANSLATE_NOOP("Backend", "Select on your smartphone");
    m_text[trGui05c] = QCoreApplication::translate("Backend", trGui05c);

    constexpr char trGui05d[] = QT_TRANSLATE_NOOP("Backend", "Excel Form Marteloscope");
    m_text[trGui05d] = QCoreApplication::translate("Backend", trGui05d);

    constexpr char trGui05e[] = QT_TRANSLATE_NOOP("Backend", "Excel Form Travailloscope");
    m_text[trGui05e] = QCoreApplication::translate("Backend", trGui05e);

    constexpr char trGui07[] = QT_TRANSLATE_NOOP("Backend", "Your feedback");
    m_text[trGui07] = QCoreApplication::translate("Backend", trGui07);

    constexpr char trGui08[] = QT_TRANSLATE_NOOP("Backend", "To improve the application");
    m_text[trGui08] = QCoreApplication::translate("Backend", trGui08);

    constexpr char trGui09[] = QT_TRANSLATE_NOOP("Backend", "Check");
    m_text[trGui09] = QCoreApplication::translate("Backend", trGui09);

    constexpr char trGui10[] = QT_TRANSLATE_NOOP("Backend", "if an update is available");
    m_text[trGui10] = QCoreApplication::translate("Backend", trGui10);

    constexpr char trGui11[] = QT_TRANSLATE_NOOP("Backend", "File saved successfully");
    m_text[trGui11] = QCoreApplication::translate("Backend", trGui11);

    constexpr char trGui12[] = QT_TRANSLATE_NOOP("Backend", "Error");
    m_text[trGui12] = QCoreApplication::translate("Backend", trGui12);

    constexpr char trGui13[] = QT_TRANSLATE_NOOP("Backend", "Score");
    m_text[trGui13] = QCoreApplication::translate("Backend", trGui13);

    constexpr char trGui14[] = QT_TRANSLATE_NOOP("Backend", "Date");
    m_text[trGui14] = QCoreApplication::translate("Backend", trGui14);

    constexpr char trGui15[] = QT_TRANSLATE_NOOP("Backend", "Travailloscope Quiz");
    m_text[trGui15] = QCoreApplication::translate("Backend", trGui15);
}
