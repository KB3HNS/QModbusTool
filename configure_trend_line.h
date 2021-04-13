/**
 * \file configure_trend_line.h
 * \brief Trend configuration dialog box
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
 * Configure a trend.
 */

#ifndef CONFIGURE_TREND_LINE_H
#define CONFIGURE_TREND_LINE_H

//  c++ includes
#include <QColor>  //  QColor

// C includes
/* -none- */

// project includes
#include "base_dialog.h"  //  BaseDialog
#include "ui_configure_trend_line.h"  //  Ui::ConfigureTrendDialog
#include "trend_line.h"  //  TrendLine
#include "trend_window.h"  //  TrendWindow


/**
 * @brief The ConfigureTrendLine class
 */
class ConfigureTrendLine : public BaseDialog
{
    Q_OBJECT

public:

    /**
     * @brief "new" trend constructor
     * @param parent parent window
     */
    ConfigureTrendLine(TrendWindow *const parent);

    /**
     * @brief "Update existing trend" constructor
     * @param parent trend to update
     */
    ConfigureTrendLine(TrendLine *const parent);

    /**
     * @brief create_trend
     * \note
     * Only to be called once the dialog is complete.
     *
     * @throw AppException if a line is already associated
     * @return Trend based on the current configuration
     */
    [[nodiscard]] TrendLine* create_trend();

    /**
     * @brief get the key (IE node<<16|reg)
     */
    operator quint32() const;

    virtual ~ConfigureTrendLine() override;

protected:
    virtual void setupUi() final override;

private slots:

    void on_Accept_pressed();
    void on_Cancel_pressed();
    void on_Delete_pressed();
    void on_ColorButton_pressed();

private:

    void update_color_labels();

    Ui::ConfigureTrendDialog *const m_ui;
    QColor m_display_color;
    TrendWindow *const m_parent;
    TrendLine *m_trend;
};


#endif // CONFIGURE_TREND_LINE_H
