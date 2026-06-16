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

#include "c_pdf.h"
#include "c_chart.h"
#include "c_result.h"
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QPdfWriter>
#include <QPainter>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QPageSize>
#include <QMarginsF>
#include <QTextBlock>
#include <QRegularExpression>

// -----------------------------------------------------------------------------
// Wraps each <img ...> in <p>...</p>
// Converts text separated by <br> / <br><br> (and variants) into <p>...</p>
// Keeps empty lines (<br><br>) as paragraph separators.
// Assumes input is "simple" HTML similar to the example (no existing <p> tags).
static QString pdfNormalizeHtmlWithParagraphs(QString html)
{
    QString pTagStart = "<p style='color: black;'>";
    QString pTagEnd = "</p>";

    auto isBr = [](const QString &s) {
        // Normalize common <br> variants to "<br/>" for internal processing
        // We'll handle <br>, <br/>, <br />, case-insensitive.
        Q_UNUSED(s);
    };

    // 1) Normalize <br> variants to "<br/>"
    html.replace(QRegularExpression(R"(<\s*br\s*/?\s*>)", QRegularExpression::CaseInsensitiveOption), "<br/>");

    // 2) Wrap each <img ...> as its own paragraph
    // Also captures <img .../> and <img ...>
    html.replace(QRegularExpression(R"((<\s*img\b[^>]*>))", QRegularExpression::CaseInsensitiveOption),
                 "<p>\\1</p>");
                 //"<p>$1</p>");

    // 3) Tokenize into "paragraph breaks" and content.
    // Treat 2+ consecutive <br/> as a paragraph separator.
    // Treat a single <br/> as a paragraph separator too (per your requirement).
    //
    // Strategy:
    // - Replace 2+ <br/> with a strong separator "\n\n"
    // - Replace 1 <br/> with separator "\n"
    // - Then split on blank lines and single newlines similarly.
    QString t = html;
    t.replace(QRegularExpression(R"((?:<br/>\s*){2,})", QRegularExpression::CaseInsensitiveOption), "\n\n");
    t.replace(QRegularExpression(R"(<br/>\s*)", QRegularExpression::CaseInsensitiveOption), "\n");

    // 4) Split into logical blocks on \n or \n\n while preserving already-wrapped <p>...</p>.
    // We'll:
    // - Split by '\n'
    // - Build paragraphs: each line becomes one paragraph; empty lines just separate paragraphs.
    const QStringList lines = t.split('\n', Qt::KeepEmptyParts);

    QString out;
    QString current;

    auto flushParagraph = [&]() {
        QString s = current.trimmed();
        current.clear();
        if (s.isEmpty())
            return;

        // If it's already a paragraph (e.g., <p><img ...></p>), keep as-is.
        // Also keep other block tags if you have them; adjust as needed.
        const bool alreadyP = s.startsWith("<p", Qt::CaseInsensitive) && s.endsWith("</p>", Qt::CaseInsensitive);
        if (alreadyP) {
            out += s;
        } else {
            out += pTagStart + s + pTagEnd;
        }
    };

    for (const QString &line : lines) {
        if (line.trimmed().isEmpty()) {
            // Empty line -> end current paragraph
            flushParagraph();
            continue;
        }

        // If a line contains one or more complete <p>...</p> blocks (e.g., from image wrapping),
        // flush any pending text, then append those blocks as standalone.
        // This keeps images separate even if adjacent to text.
        const QRegularExpression pBlockRe(
            R"((<p\b[^>]*>.*?</p>))",
            QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);

        QRegularExpressionMatchIterator it = pBlockRe.globalMatch(line);
        if (it.hasNext()) {
            flushParagraph();

            int last = 0;
            while (it.hasNext()) {
                auto m = it.next();
                const int start = m.capturedStart(1);
                const int end = m.capturedEnd(1);

                // Any text before the <p> block becomes its own paragraph
                const QString before = line.mid(last, start - last).trimmed();
                if (!before.isEmpty())
                    out += pTagStart + before + pTagEnd;

                out += m.captured(1);
                last = end;
            }

            const QString after = line.mid(last).trimmed();
            if (!after.isEmpty())
                out += pTagStart + after + pTagEnd;

            continue;
        }

        // Normal text line: accumulate into current paragraph (you can choose to join lines with space)
        if (!current.isEmpty())
            current += " ";
        current += line.trimmed();
    }

    flushParagraph();

    // Optional: wrap with html/body and normalize spacing
    // return "<html><body>" + out + "</body></html>";
    return out;
}

static QVector<qreal> pdfComputePageStartsByBlocks(const QTextDocument &doc, qreal pageHeight)
{
    QVector<qreal> starts;
    starts.push_back(0.0);

    auto *layout = doc.documentLayout();

    qreal yPageStart = 0.0;
    qreal yLimit     = pageHeight; // yPageStart + pageHeight

    for (QTextBlock b = doc.begin(); b.isValid(); b = b.next()) {
        const QRectF br = layout->blockBoundingRect(b);
        const qreal top = br.top();
        const qreal bottom = br.bottom();
        const qreal h = br.height();

        // If this block fits on the current page, continue.
        if (bottom <= yLimit + 0.0001)
            continue;

        // We need a page break before/within this block.
        if (h <= pageHeight + 0.0001) {
            // Normal case: move whole block to next page
            yPageStart = top;
            starts.push_back(yPageStart);
            yLimit = yPageStart + pageHeight;
        } else {
            // Oversized block: it cannot fit on one page.
            // 1) Start a new page at the block top (avoid duplicating part of the block above top)
            if (top > yPageStart + 0.0001) {
                yPageStart = top;
                starts.push_back(yPageStart);
                yLimit = yPageStart + pageHeight;
            }

            // 2) Then add regular page breaks inside the block
            while (bottom > yLimit + 0.0001) {
                yPageStart = yLimit;          // next page starts exactly at previous limit
                starts.push_back(yPageStart);
                yLimit = yPageStart + pageHeight;
            }
        }
    }

    // Optional: de-duplicate if two starts are extremely close
    // (can happen with floating point edge cases)
    QVector<qreal> out;
    out.reserve(starts.size());
    for (qreal s : starts) {
        if (out.isEmpty() || qAbs(out.last() - s) > 0.01)
            out.push_back(s);
    }
    return out;
}

static void pdfPrintDocument(QTextDocument &doc, QPdfWriter &writer)
{
    const QRectF pageRect = writer.pageLayout().paintRectPoints(); // points

    doc.setPageSize(pageRect.size());
    doc.setTextWidth(pageRect.width());

    doc.documentLayout()->documentSize();

    const QVector<qreal> starts = pdfComputePageStartsByBlocks(doc, pageRect.height());

    QPainter p(&writer);

    for (int i = 0; i < starts.size(); ++i)
    {
        if (i > 0)
        {
            writer.newPage();
        }

        const qreal yStart = starts[i];
        const qreal yEnd = (i + 1 < starts.size())
            ? starts[i + 1]
            : (yStart + pageRect.height());

        const qreal clipH = qMin(pageRect.height(), yEnd - yStart);

        p.save();
        p.translate(pageRect.left(), pageRect.top());
        p.setClipRect(QRectF(0, 0, pageRect.width(), clipH));
        p.translate(0, -yStart);
        doc.drawContents(&p);
        p.restore();
    }

    p.end();
}

static QString pdfHeaderToHtml(const QString &text)
{
    return "<span style='font-weight:700;color:#26787A;'>" + text + "</span>";
}

static QString pdfHr()
{
    return "<hr style='border: none; height: 2px; background-color: #26787A;'>";
}

static QString pdfExportHtml(const QUrl &targetUrl,
                             const QString &header,
                             QTextDocument &doc,
                             QString &html,
                             Data &data,
                             int imageAutoWidth)
{
    const QString urlString = targetUrl.toString();

    QFile out(urlString);
    if (!out.open(QIODevice::WriteOnly))
    {
        return "Cannot create the file";
    }

    // writer
    QPdfWriter writer(&out);
    writer.setPageLayout(QPageLayout(QPageSize(QPageSize::A4),
                                     QPageLayout::Portrait,
                                     QMarginsF(1, 1, 1, 1),
                                     QPageLayout::Millimeter));

    writer.setResolution(72);
    const QRect pageRect = writer.pageLayout().paintRectPoints();
    int pageWidth = pageRect.width();

    // Title
    QString siteName = data.loadedExerciseName().section(' ', 0, 0);
    QDate date = QDate::currentDate();
    QString logo1Width = QString::number((pageWidth / 100) * 60);
    QString logo2Width = QString::number((pageWidth / 100) * 10);
    QString img1 = "<img src='qrc:/images/ui_logo_partners.png' width='" + logo1Width + "'/>";
    QString img2 = "<img src='qrc:/images/ui_logo_co_funded_by_eu.jpg' width='" + logo2Width + "'/>";
    QString hdr1Name = data.text(header) + " : " + siteName;
    QString hdr1Date = data.text("Date") + " : " + date.toString("dd/MM/yyyy");
    QString title = "<table width='100%'>"
                    "  <tr>"
                    "    <td align='left'>" + img1 + "</td>"
                    "    <td align='right'>" + img2 + "</td>"
                    "  </tr>"
                    "  <tr>"
                    "    <td align='left'>" + pdfHeaderToHtml(hdr1Name) + "</td>"
                    "    <td align='right'>" + hdr1Date + "</td>"
                    "  </tr>"
                    "</table>";

    html = title + html;

    // doc
    QFont f("Sans");
    f.setPointSize(11);
    doc.setDefaultFont(f);
    doc.setDefaultStyleSheet("body { color: blue; }");

    data.formatTextImageSize(html, pageWidth, pageWidth / imageAutoWidth);

    doc.setHtml(html);

    pdfPrintDocument(doc, writer);

    out.close();

    return QString();
}

// -----------------------------------------------------------------------------
QString Pdf::exportTravailloscopeQuiz(const QUrl &targetUrl, Data &data)
{
    QString siteName = data.loadedExerciseName().section(' ', 0, 0);

    const TravailloscopeQuiz &quiz = data.travailloscopeQuiz();
    const auto spotIdList = quiz.spotIdList();

    QTextDocument doc;
    QString html;

    for (auto spotId : spotIdList)
    {
        html += pdfHr();
        html += "<b>" + siteName + " " + quiz.spotNo(spotId) + "</b>";
        html += quiz.pedagogicalContents(spotId, 0);
        html += "<br><br>";
    }

    html += quiz.scoreTable();

    html = pdfNormalizeHtmlWithParagraphs(html);

    return pdfExportHtml(targetUrl, "Travailloscope Quiz", doc, html, data, 3);
}

// -----------------------------------------------------------------------------
class PdfResultsChapter
{
public:
    QString number;
    double imageWidth;
    QString header;
    QString imageSource;
    QString mainHeader;
};

static void pdfProcessResultsChapter(const PdfResultsChapter &chapter,
                                     Data &data,
                                     const Chart::Options &options,
                                     QTextDocument &doc,
                                     std::vector<QString> &chapters,
                                     std::vector<QImage> &images)
{
    const QString &header = chapter.header;
    const QString &imageSource = chapter.imageSource;
    double imageWidth = chapter.imageWidth;

    // image
    double imageHeight = Chart::imageHeight(imageWidth, imageSource, data, options);

    int w = static_cast<int>(imageWidth);
    int h = static_cast<int>(imageHeight);

    QImage image(w, h, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    Chart::requestImage(painter, imageSource, data, options);

    QUrl imageUrl = "image://dyn/img" + QString::number(images.size());

    images.push_back(image);

    doc.addResource(QTextDocument::ImageResource, imageUrl, image);

    QString imageWidthStr;
    if (imageSource != "mapImage")
    {
        if (imageSource == "qualityEvolutionImage")
        {
            imageWidthStr = " width='200'";
        }
        else
        {
            imageWidthStr = " width='1'";
        }
    }

    // html parts
    QString mainHeaderHtml;
    if (!chapter.mainHeader.isEmpty())
    {
        mainHeaderHtml = pdfHeaderToHtml(data.text(chapter.mainHeader)) + "<br>";
    }

    QString headerImageHtml;
    if (!chapter.number.isEmpty())
    {
        headerImageHtml = "<img src='qrc:/images/ui_number_" + chapter.number + ".png' width='30'/>";
    }

    QString headerTextHtml = pdfHeaderToHtml(data.text(header)) + "<br>";

    QString chapterImageHtml = QString("<img src='%1'" + imageWidthStr + "/>").arg(imageUrl.toString());

    // html
    QString html;
    html += "<p style='margin:0; page-break-inside:avoid;'>";
    html += mainHeaderHtml;
    html += headerImageHtml;
    html += headerTextHtml;
    html += chapterImageHtml;
    html += "</p>";
    html += "<br>";

    chapters.push_back(html);
}

QString Pdf::exportMarteloscopeResults(const QUrl &targetUrl,
                                       Data &data,
                                       const Chart::Options &options)
{
    // Custom options for PDF
    Chart::Options customOptions = options;
    customOptions.showAllTreeStatisticsSpecies = false;

    // PDF image resolution in px
    double imgWidth = 1000.0;
    double mapWidth = 2000.0;

    // Map : reset view
    View mapViewOld = data.view();
    data.view().setDimensions(mapWidth, mapWidth, 72.0);
    data.view().setZoom(data.view().zoomMin(), 1.0, 0.5, 0.5);
    data.view().setShowMapTreeSpeciesLegend(true);
    data.view().setMoveMapTreeSpeciesLegend(false);
    data.view().resetTranslation();

    // Chapters
    std::vector<PdfResultsChapter> chapters = {
        {   "", mapWidth, "Results: Map", "mapImage" },

        { "01", imgWidth, "Results: Overall CCF assessment Header", "overallAssessmentImage" },
        { "02", imgWidth, "Results: Tree Statistics Header", "treeStatisticsImage" },
        { "03", imgWidth, "Results: Economic Values Header", "economicUserValuesImage" },
        { "04", imgWidth, "Results: Economic Values (Expert) Header", "economicExpertValuesImage" },
        { "05", imgWidth, "Results: Tree Distribution Header", "treeDistributionImage" },
        { "06", imgWidth, "Results: Gini Index Header", "giniIndexImage" },
        { "07", imgWidth, "Results: K Factor Header", "thinningFactorImage" },
        { "08", imgWidth, "Results: Quality - before Header", "qualityBeforeImage" },
        { "08", imgWidth, "Results: Quality - cut Header", "qualityCutImage" },
        { "08", imgWidth, "Results: Quality - after Header", "qualityAfterImage" },
        { "09", imgWidth, "Results: Quality Evolution Header", "qualityEvolutionImage" },
        { "10", imgWidth, "Results: Composition - before Header", "compositionBeforeImage" },
        { "10", imgWidth, "Results: Composition - cut Header", "compositionCutImage" },
        { "10", imgWidth, "Results: Composition - after Header", "compositionAfterImage" },
        { "11", imgWidth, "Results: Shannon Index Header", "shannonIndexImage" },
        { "12", imgWidth, "Results: Choices Assessment Header", "choicesAssessmentImage" },
        { "13", imgWidth, "Results: Dead and Biological Trees Header", "ecologicalValuesDeadBioImage" },
        { "14", imgWidth, "Results: Prosilva Header", "ecologicalValuesImage" },
        { "15", imgWidth, "Results: Dendromicrohabitats (DMH) Header", "ecologicalValuesDmhImage" },

        { "01", imgWidth, "Results: Local Typology Header", "typologyLocalImage", "Results: Marteloscope Advanced Results" },
        { "02", imgWidth, "Results: Global Typology Header", "typologyGlobalImage" },
        { "03", imgWidth, "Results: Overall CCF assessment Header", "overallAssessmentImage" }
    };

    QTextDocument doc;
    std::vector<QString> chaptersHtml;
    std::vector<QImage> images;

    for (size_t i = 0; i < chapters.size(); i++)
    {
        pdfProcessResultsChapter(chapters[i], data, options, doc, chaptersHtml, images);
    }

    // html
    QString html;
    html += pdfHr();
    for (size_t i = 0; i < chaptersHtml.size(); i++)
    {
        html += chaptersHtml[i];
    }

    // Map : restore view
    data.view() = mapViewOld;

    return pdfExportHtml(targetUrl, "Results: Marteloscope Results", doc, html, data, 2);
}
// -----------------------------------------------------------------------------
