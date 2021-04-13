/**
 * \file trend_line.cpp
 * \brief Individual trend line
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
#include <limits>  //  std::numeric_limits
#include <QStringBuilder>  //  operator%
#include <algorithm>  //  std::fill

// C includes
/* -none- */

// project includes
#include "trend_line.h"  //  local include
#include "exceptions.h"  //  AppException
#include "configure_trend_line.h"  //  ConfigureTrendLine


TrendLine::TrendLine(TrendWindow *parent,
                     const quint16 reg,
                     const quint8 node) :
        QPushButton(parent->m_scroll_container),
        m_reg_number{reg},
        m_device_id{node},
        m_num_points{parent->m_timestamps.size()},
        m_signed_value{true},
        m_mult{1.0},
        m_offset{0.0},
        m_pen_color(Qt::blue),
        m_history(m_num_points),
        m_last_value(),
        m_next_index{0},
        m_parent{parent}
{
    connect(this, &TrendLine::clicked, this, &TrendLine::on_clicked);
    setupUi();
}


void TrendLine::set_data(const quint16 value) noexcept
{
    if (m_signed_value) {
        const auto v = static_cast<qint16>(value);
        m_last_value = qint32(v);
    } else {
        m_last_value = qint32(value);
    }
}


void TrendLine::update()
{
    if (!bool(m_last_value)) {
        throw AppException(tr("Update called on invalid data"));
    }

    auto v = double(m_last_value.value()) * m_mult;
    v += m_offset;
    m_parent->update_min_max(v);
    m_history[m_next_index] = v;

    if (++m_next_index >= m_num_points) {
        m_next_index = 0;
    }

    m_last_value.reset();
}


TrendLine::operator bool() const noexcept
{
    return m_last_value.has_value();
}


const double& TrendLine::operator[] (int index) const
{
    auto array_index = m_next_index + index;
    if (array_index >= m_num_points) {
        array_index -= m_num_points;
    }

    return m_history.at(array_index);
}


void TrendLine::configure(const double m, const double b, const bool set_signed)
{
    m_mult = m;
    m_offset = b;
    m_signed_value = set_signed;
}


void TrendLine::set_color(const QColor &pen_color) noexcept
{
    m_pen_color = pen_color;
    setupUi();
}


TrendLine::operator quint32() const noexcept
{
    return m_parent->get_key(m_reg_number, m_device_id);
}


TrendLine::operator QPen() const noexcept
{
    return QPen(m_pen_color);
}


void TrendLine::on_clicked()
{
    auto dlg = ConfigureTrendLine(this);
    dlg.exec();
}


void TrendLine::setupUi()
{
    setFixedWidth(m_parent->m_add_button->size().width());
    setAutoDefault(false);
    const auto reg_number = QString::number(m_reg_number);
    const auto reg_padding = QString(5 - reg_number.size(), QChar('0'));
    setText("------\n" % reg_padding %
            reg_number % QChar('@') % QString::number(m_device_id));
    setStyleSheet("QPushButton {color: " % m_pen_color.name() % ";}");
}


void TrendLine::save_configuration(QDomElement &node) const
{
    node.setAttribute("register", QString::number(m_reg_number));
    node.setAttribute("node", QString::number(m_device_id));
    node.setAttribute("signed", QString::number(int(m_signed_value)));
    node.setAttribute("m", QString::number(m_mult));
    node.setAttribute("b", QString::number(m_offset));
    node.setAttribute("color", m_pen_color.name());

}


void TrendLine::resize(int new_size)
{
    m_next_index = 0;
    m_num_points = new_size;
    m_history.resize(new_size);
    std::fill(m_history.begin(), m_history.end(), m_offset);
}
