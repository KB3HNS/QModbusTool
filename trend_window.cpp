/**
 * \file trend_window.cpp
 * \brief Graphing / Trend support
 * \copyright
 * 2021 Andrew Buettner (ABi)
 *
 * \section LICENSE
 *
 * QModbusTool - A QT Based Modbus Client
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


//  c++ includes
#include <QTimer>  //  QTimer
#include <QMessageBox>  //  QMessageBox
#include <QAction>  //  QAction
#include <QFileDialog>  //  QFileDialog
#include <qtcsv/stringdata.h>  //  QtCSV::StringData
#include <qtcsv/writer.h>  //  QtCSV::Writer::write

// C includes
/* -none- */

// project includes
#include "trend_window.h"  //  local include
#include "trend_line.h"  //  TrendLine
#include "configure_trend_line.h"  //  ConfigureTrendLine
#include "configure_trend.h"  //  ConfigureTrend


using TimeDiff = std::chrono::duration<double>;
using std::chrono::steady_clock;


namespace {
    const auto g_num_points = 100;
}


TrendWindow::TrendWindow(QWidget *const parent) :
    BaseDialog(parent, false),
    m_plot{new QCustomPlot(this)},
    m_data(),
    m_timestamps(),
    m_start_time{steady_clock::now()},
    m_layout{new QHBoxLayout(this)},
    m_legend{new QGroupBox(tr("Legend:"), this)},
    m_legend_layout{new QVBoxLayout(m_legend)},
    m_button_area{new QScrollArea(m_legend)},
    m_scroll_container{new QWidget(m_button_area)},
    m_scroll_layout{new QVBoxLayout(m_scroll_container)},
    m_add_button{new QPushButton(tr("Add new\nregister"), m_scroll_container)},
    m_configure_button{new QPushButton(tr("Graph\nMenu"), m_legend)},
    m_main_menu{new QMenu(m_configure_button->text(), m_configure_button)},
    m_miny{0.0},
    m_maxy{1.0}
{
    auto timepoint = 0.0;
    for (auto i=g_num_points; i>0; --i) {
        --timepoint;
        m_timestamps.prepend(timepoint);
    }

    setLayout(m_layout);
    connect(m_add_button, &QPushButton::clicked, this, &TrendWindow::on_add_button_clicked);
}


void TrendWindow::setupUi()
{
    /* Configure basic window layout and top-level widgets */
    m_layout->addWidget(m_legend);
    m_legend->setLayout(m_legend_layout);
    m_legend->setFixedWidth(160);
    m_legend_layout->addSpacing(15);
    m_legend_layout->addWidget(m_configure_button);

    /* Menu Button and menu */
    m_configure_button->setAutoDefault(false);
    m_configure_button->setMenu(m_main_menu);
    auto configure = m_main_menu->addAction(tr("Configure Graph"));
    connect(configure, &QAction::triggered, this, &TrendWindow::on_configure_triggered);
    auto save = m_main_menu->addAction(tr("Save Data"));
    connect(save, &QAction::triggered, this, &TrendWindow::on_save_triggered);
    auto capture = m_main_menu->addAction(tr("Save Screenshot"));
    connect(capture, &QAction::triggered, this, &TrendWindow::on_capture_triggered);

    /* Scrollable "Legend" containing a list of graphs */
    m_legend_layout->addWidget(m_button_area);
    m_button_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_button_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_button_area->setWidget(m_scroll_container);
    m_scroll_container->setLayout(m_scroll_layout);
    m_button_area->setFrameShape(QFrame::NoFrame);
    m_button_area->setFrameRect(QRect(0, 0, 0, 0));
    m_button_area->setWidgetResizable(true);
    m_scroll_layout->setAlignment(Qt::AlignTop);
    m_scroll_layout->addWidget(m_add_button);
    m_add_button->setFixedWidth(125);
    m_add_button->setDefault(true);

    /* Build the actual plot.  It must have at least 1 graph. */
    m_layout->addWidget(m_plot);
    m_plot->addGraph();
    m_plot->xAxis->setLabel(tr("time"));
    m_plot->yAxis->setLabel(tr("value"));
    m_plot->setOpenGl(true);

    resize(850, 340);

    setWindowTitle(tr("Trend"));
}


void TrendWindow::on_new_value(const quint16 reg,
                               const quint16 value,
                               const quint8 unit_id)
{
    auto graph_inst = m_data.find(get_key(reg, unit_id));
    if (m_data.end() != graph_inst) {
        graph_inst->second->set_data(value);
        scan();
    }
}


void TrendWindow::redraw_graph()
{
    auto time = m_timestamps.begin();
    const auto start_time = *time;
    const auto graph_size = m_timestamps.size();
    auto end_time = std::numeric_limits<double>::max();

    QVector<double> x(graph_size);
    for (auto i=0; i < graph_size; ++i) {
        end_time = *time;
        x[i] = end_time;
        ++time;
    }

    QVector<double> y(graph_size, 0.0);
    auto graph = 0;

    for (const auto &line: m_data) {
        for (auto i=0; i<graph_size; ++i) {
            y[i] = (*line.second)[i];
        }
        m_plot->graph(graph)->setData(x, y, true);
        m_plot->graph(graph)->setPen(QPen(*line.second));
        ++graph;
    }

    if (0 == graph) {
        m_plot->graph(0)->setData(x, y, true);
    }

    m_plot->xAxis->setRange(start_time, end_time);
    m_plot->yAxis->setRange(m_miny, m_maxy);
    m_plot->replot();
}


void TrendWindow::scan()
{
    for (const auto &i: m_data) {
        if (!bool(*(i.second))) {
            return;
        }
    }

    const TimeDiff diff = steady_clock::now() - m_start_time;
    m_timestamps.removeFirst();
    m_timestamps.append(diff.count());

    for (auto &i: m_data) {
        i.second->update();
    }

    redraw_graph();
}


quint32 TrendWindow::get_key(const quint16 reg, const quint8 node) const noexcept
{
    return (quint32(node) << 16U) | quint32(reg);
}


void TrendWindow::add_trend(TrendLine *const trend)
{
    m_data[quint32(*trend)] = trend;
    if (size_t(m_plot->graphCount()) < m_data.size()) {
        m_plot->addGraph();
    }

    auto w = m_scroll_layout->count();
    m_scroll_layout->insertWidget(w - 1, trend);

    redraw_graph();
}


void TrendWindow::on_add_button_clicked()
{
    auto dlg = ConfigureTrendLine(this);
    if (dlg.exec() != 0) {
        auto key = quint32(dlg);
        const auto trend = m_data.find(key);
        if (m_data.end() != trend) {
            auto error_box = QMessageBox(this);
            error_box.setIcon(QMessageBox::Critical);
            error_box.setStandardButtons(QMessageBox::Ok);
            error_box.setText(tr("Error"));
            error_box.setWindowTitle(tr("Invalid Configuration"));
            error_box.setInformativeText(tr("Duplicate trend requested.\n"
                                            "Edit trend instead"));
            error_box.setDetailedText(tr("Register: %1\nRemote Node: %2")
                                      .arg(trend->second->m_reg_number)
                                      .arg(trend->second->m_device_id));
            error_box.exec();
        } else {
            add_trend(dlg.create_trend());
        }
    }
}


void TrendWindow::remove_trend(const quint32 trend_key)
{
    auto trend_index = m_data.find(trend_key);
    auto trend = trend_index->second;
    m_data.erase(trend_index);
    m_scroll_layout->removeWidget(trend);
    const auto graph_count = m_plot->graphCount();
    if (graph_count > 1) {
        m_plot->removeGraph(graph_count - 1);
    }

    redraw_graph();

    //  Don't let it happen until the next entry into the scheduler as the
    // instance is probably in our call path.
    emit trend->deleteLater();
}


void TrendWindow::on_configure_triggered()
{
    auto dlg = ConfigureTrend(this, m_miny, m_maxy, m_timestamps.size());
    if (dlg.exec() != 0) {
        m_fixed_limits = bool(dlg);

        const auto limits = dlg.get_min_max();
        m_miny = limits.first;
        m_maxy = limits.second;

        const auto points = dlg.get_num_points();
        if (points.has_value()) {
            resize_history(points.value());
        }
    }
}


void TrendWindow::on_save_triggered()
{
    auto dialog = QFileDialog(this, tr("Save data as..."));
    dialog.setNameFilters({tr("Spreadsheet (*.csv)"),
                           tr("All files (*)")});
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setDefaultSuffix("csv");
    if (dialog.exec() != 0) {
        auto file_name = dialog.selectedFiles().first();
        if (!save_register_set(file_name)) {
            QMessageBox::warning(this, tr("Save history"),
                                 tr("Cannot write file %1:\n.")
                                 .arg(QDir::toNativeSeparators(file_name)));
        }
    }
}


void TrendWindow::on_capture_triggered()
{
    auto dialog = QFileDialog(this, tr("Save image as..."));
    dialog.setNameFilters({tr("PNG (*.png)"),
                           tr("JPG (*.jpg)"),
                           tr("BMP (*.bmp)")});
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setDefaultSuffix("png");
    if (dialog.exec() != 0) {
        auto file_name = dialog.selectedFiles().first();
        const auto ext = file_name.split(".").back().toLower();
        auto status = true;
        if (ext == "png") {
            status = m_plot->savePng(file_name);
        } else if (ext == "jpg") {
            status = m_plot->saveJpg(file_name);
        } else if (ext == "bmp") {
            status = m_plot->saveBmp(file_name);
        } else {
            QMessageBox::warning(this, tr("Save Image"),
                                 tr("Unknown extension: %1")
                                 .arg(ext));
        }

        if (!status) {
            QMessageBox::warning(this, tr("Save Image"),
                                 tr("Error saving image: %1")
                                 .arg(QDir::toNativeSeparators(file_name)));
        }
    }
}


void TrendWindow::update_min_max(const double value) noexcept
{
    if (!m_fixed_limits) {
        if (value > m_maxy) {
            m_maxy = value;
        } else if (value < m_miny) {
            m_miny = value;
        } else {

        }
    }
}


bool TrendWindow::save_register_set(const QString &path)
{
    QStringList header;
    header << tr("Register number") << tr("Device ID/Node") << tr("Line Color");

    for (const auto i: m_timestamps) {
        header << QString::number(i);
    }
    QtCSV::StringData csv_data;
    csv_data.addRow(header);

    for (const auto &i: m_data) {
        QStringList row;
        row << QString::number(i.second->m_reg_number)
            << QString::number(i.second->m_device_id)
            << QPen(*(i.second)).color().name();


        for (auto index=0; index<m_timestamps.size(); ++index) {
            row << QString::number((*(i.second))[index]);
        }
        csv_data.addRow(row);
    }

    return QtCSV::Writer::write(path, csv_data);
}


QDomElement TrendWindow::save_configuration(QDomDocument &root) const
{
    auto trend = root.createElement("trend");
    const auto position = pos();
    const auto w_size = size();
    trend.setAttribute("w", QString::number(w_size.width()));
    trend.setAttribute("h", QString::number(w_size.height()));
    trend.setAttribute("x", QString::number(position.x()));
    trend.setAttribute("y", QString::number(position.y()));
    trend.setAttribute("min", QString::number(m_miny));
    trend.setAttribute("max", QString::number(m_maxy));
    trend.setAttribute("fixed", QString::number(int(m_fixed_limits)));
    trend.setAttribute("points", QString::number(m_timestamps.size()));

    for (const auto &i: m_data) {
        auto line = root.createElement("trend_line");
        i.second->save_configuration(line);
        trend.appendChild(line);
    }

    return trend;
}


bool TrendWindow::load_configuration(const QDomElement &node)
{
    if (m_data.size() > 0) {
        return false;
    }

    bool okmin, okmax;
    const auto w = node.attribute("w", "-1").toInt();
    const auto h = node.attribute("h", "-1").toInt();
    const auto points = node.attribute("points", "-1").toInt();
    const auto fixed = node.attribute("fixed", "-1").toInt();
    const auto x = node.attribute("x", "");
    const auto y = node.attribute("y", "");
    const auto min = node.attribute("min", "[bad]").toDouble(&okmin);
    const auto max = node.attribute("max", "[bad]").toDouble(&okmax);

    if ((w <= 0) || (h <= 0) || (points <= 0) || (fixed < 0) ||
            (x.size() == 0) || (y.size() == 0) || !okmin || !okmax) {
        return false;
    }

    resize_history(points, false);
    m_fixed_limits = bool(fixed);
    m_miny = min;
    m_maxy = max;

    QVector<TrendLine*> new_lines;
    const auto &children = node.childNodes();
    new_lines.reserve(children.count());
    for (auto i=0; i<children.count(); ++i) {
        const auto &node = children.at(i);
        if (node.isElement() && node.nodeName() == "trend_line") {
            const auto &element = node.toElement();

            const auto reg = element.attribute("register", "-1").toInt();
            const auto node = element.attribute("node", "-1").toInt();
            const auto is_signed = element.attribute("signed", "-1").toInt();
            bool okm, okb;
            const auto m = element.attribute("m", "[bad]").toDouble(&okm);
            const auto b = element.attribute("b", "[bad]").toDouble(&okb);
            const auto color = QColor(element.attribute("color", "[bad]"));

            if ((reg < 1) || (node < 0) || (is_signed < 0) || !okm || !okb) {
                return false;
            }

            auto line = new TrendLine(this, quint16(reg), quint8(node));
            new_lines.append(line);

            line->configure(m, b, bool(is_signed));
            line->set_color(color);
        }
    }

    QTimer::singleShot(160, this, [=]() {
        resize(w, h);
        move(x.toInt(), y.toInt());
        for (auto i: new_lines) {
            add_trend(i);
        }
    });

    return true;
}


void TrendWindow::resize_history(const int points, bool redraw)
{
    while (m_timestamps.size() > points) {
        m_timestamps.removeFirst();
    }

    auto earliest = m_timestamps.first();
    while (m_timestamps.size() < points) {
        --earliest;
        m_timestamps.prepend(earliest);
    }

    for (auto &i: m_data) {
        i.second->resize(points);
    }

    if (redraw) {
        redraw_graph();
    }
}
