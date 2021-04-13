/**
 * \file trend_line.h
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
 *
 * \section DESCRIPTION
 *
 * The trend data is encapsulated within the "Button" located in the "Legend"
 * section of the graph.  It overlads the actual button and provides a bunch of
 * syntactic sugar to interface to the underlying trend data.  This class shall
 * not be used outside of the TrendWindow class.
 */

#ifndef TREND_LINE_H
#define TREND_LINE_H

//  c++ includes
#include <QVector>  //  QVector
#include <QColor>  //  QColor
#include <QPushButton>  //  QPushButton
#include <QPen>  //  QPen
#include <QDomElement>  //  QDomElement
#include <optional>  //  std::optional

// C includes
/* -none- */

// project includes
#include "trend_window.h"


/**
 * @brief Graph indivdual line configuration and storage
 */
class TrendLine : public QPushButton
{
    friend class ConfigureTrendLine;

    Q_OBJECT

public:

    /**
     * @brief constructor
     * @param parent parent TrendWindo
     * @param reg modbus register to monitor
     * @param node node / device ID to monitor
     */
    TrendLine(TrendWindow *parent, const quint16 reg, const quint8 node=0);

    /**
     * @brief Set the current data
     * @param value current data value
     */
    void set_data(const quint16 value) noexcept;

    /**
     * @brief configure trend internals
     * @param m multiplier
     * @param b offset
     * @param set_signed interpret register as signed (``true``)
     *        or unsigned (``false``)
     */
    void configure(const double m, const double b, const bool set_signed=true);

    /**
     * @brief Set the trend line color
     * @param pen_color line color
     */
    void set_color(const QColor &pen_color) noexcept;

    /**
     * @brief Update history with current value
     * \note
     * This also updates the parent min/max values
     *
     * @throw AppException if no value has been stored.
     */
    void update();

    /**
     * @brief get current state : IE is there a value currently stored?
     */
    [[nodiscard]] operator bool() const noexcept;

    /**
      * \brief Get the QPen for graph drawing
      */
    [[nodiscard]] operator QPen() const noexcept;

    /**
      * \brief get hash key from TrendLine instance
      */
    [[nodiscard]] operator quint32() const noexcept;

    /**
     * @brief Get the value at an index
     * @param index index: 0 => oldest, g_num_points-1 => newest
     * @return historical value
     */
    [[nodiscard]] const double& operator[] (int index) const;

    /**
     * @brief Save trend line configuration
     * @param node fill data in node
     */
    void save_configuration(QDomElement &node) const;

    /**
     * @brief Resize the number of history items
     * @param new_size new number of history items
     */
    void resize(int new_size);

    const quint16 m_reg_number; /**< Register number associated with line */
    const quint8 m_device_id; /**< Device ID to monitor */

private slots:

    /**
     * @brief On button clicked handler
     */
    void on_clicked();

private:

    /**
     * @brief Update the UI (Button contents)
     */
    void setupUi();

    int m_num_points;
    bool m_signed_value; /**< Treat incoming data as signed? */
    double m_mult; /**< Multiply value by m */
    double m_offset; /**< Add b to value after multiplication */

    QColor m_pen_color; /**< Desired pen color */
    QVector<double> m_history;
    std::optional<qint32> m_last_value;
    qint32 m_next_index;
    TrendWindow *const m_parent;
};


#endif // TREND_LINE_H
