/**
 * \file configure_trend.h
 * \brief Trend graph configuration dialog box
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
 * Configure the trend graph.
 */

#ifndef CONFIGURE_TREND_H
#define CONFIGURE_TREND_H

//  c++ includes
#include <utility>  //  std::pair
#include <optional>  //  std::optional

// C includes
/* -none- */

// project includes
#include "base_dialog.h"  //  BaseDialog
#include "ui_configure_trend.h"  //  Ui::ConfigureTrend


/**
 * @brief Dialog box presenting the graph configuration options.
 */
class ConfigureTrend : public BaseDialog
{
    Q_OBJECT

public:

    /**
     * @brief constructor
     * @param parent parent widget
     * @param min graph minimum
     * @param max graph maximum
     * @param n_points number of points in graph
     */
    ConfigureTrend(QWidget *const parent,
                   const double min,
                   const double max,
                   const int n_points);

    /**
     * @brief Get min and max values.
     * @return min,max specified in controls
     */
    [[nodiscard]] std::pair<double, double> get_min_max() const;

    /**
     * @brief Get the number of points from the control.
     * @return Number of points.  Null (no value) if the value hasn't changed.
     */
    [[nodiscard]] std::optional<int> get_num_points() const;

    /**
     * @brief Get the "Use fixed limits" status.
     */
    operator bool() const;

    virtual void accept() override;

    virtual ~ConfigureTrend() override;

protected:
    virtual void setupUi() final override;

private slots:

    /**
     * @brief Action taken when the History Size control is edited.
     */
    void on_HistoryInput_textEdited();

private:
    Ui::ConfigureTrend *const m_ui;
    const int m_initial_points;
    const double m_initial_min;
    const double m_initial_max;
    bool m_first_edit;
};

#endif // CONFIGURE_TREND_H
