/**
 * \file trend_window.h
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
 *
 * \section DESCRIPTION
 *
 * The project may include 1 trend window with many trend lines updated from
 * polling data.
 */

#ifndef TREND_WINDOW_H
#define TREND_WINDOW_H

//  c++ includes
#include <QList>  //  QList
#include <QHBoxLayout>  //  QHBoxLayout
#include <QScrollArea>  //  QScrollArea
#include <QPushButton>  //  QPushButton
#include <QGroupBox>  //  QGroupBox
#include <QMenu>  //  QMenu
#include <QDomElement>  //  QDomElement
#include <QDomDocument>  //  QDomDocument
#include <qcustomplot.h>  //  QCustomPlot
#include <chrono>  //  std::chrono
#include <unordered_map>  //  std::unordered_map

// C includes
/* -none- */

// project includes
#include "base_dialog.h"


// /////////////////////////////////////////////////////////////////////////////
// Forward declarations
// /////////////////////////////////////////////////////////////////////////////
class TrendLine;


/**
 * \brief Graphing window
 */
class TrendWindow : public BaseDialog
{
    //  Allow individual trends to access internal widgets to self-configure.
    friend class TrendLine;

    Q_OBJECT

public:

    /**
     * \brief constructor
     * @param parent parent QObject owner
     */
    TrendWindow(QWidget *const parent);

    /**
      * \brief get hash key from reg, node
      * @param reg register number
      * @param node node/device ID
      */
    [[nodiscard]] quint32 get_key(const quint16 reg, const quint8 node) const noexcept;

    /**
     * @brief remove_trend
     * \note
     * This function shall only be called by the ConfigureTrendLine dialog box.
     *
     * @param trend_key
     */
    void remove_trend(const quint32 trend_key);

    /**
     * @brief Update Min/Max value
     * \note
     * This function is only called by connected trends
     *
     * @param value new value
     */
    void update_min_max(const double value) noexcept;

    /**
     * @brief save_configuration
     * @param root
     * @return
     */
    QDomElement save_configuration(QDomDocument &root) const;

    /**
     * @brief load_configuration
     * @param node
     * @return
     */
    bool load_configuration(const QDomElement &node);

public slots:

    virtual void on_new_value(const quint16 reg, const quint16 value, const quint8 unit_id) override;

protected:

    /**
     * @brief Re-draw the graph from class data
     */
    void redraw_graph();

    /**
     * @brief Scan current data set to see if it's complete.  If it is, update
     *        and re-draw the graph.
     */
    void scan();

    /**
     * @brief Add trend to the graph
     * @param trend newly created trend
     */
    void add_trend(TrendLine *const trend);

    virtual void setupUi() override;

    QCustomPlot *const m_plot;
    std::unordered_map<quint32, TrendLine*> m_data;
    QList<double> m_timestamps;
    const std::chrono::time_point<std::chrono::steady_clock> m_start_time;

private slots:

    /**
     * @brief Action when the "Add new trend" button is clicked
     */
    void on_add_button_clicked();

    /**
     * @brief Action taken when the "Configure Graph" menu item is selected
     */
    void on_configure_triggered();

    /**
     * @brief Action taken when the "Save data" menu item is selected
     */
    void on_save_triggered();

    /**
     * @brief Action taken when the "Save image" menu item is selected
     */
    void on_capture_triggered();

private:

    /**
     * \brief Save register data to CSV file
     * @param path absolute path and file name to save to
     */
    bool save_register_set(const QString &path);

    /**
     * @brief Resize the history
     * @param points new number of points to include in history
     * @param redraw set ``false`` to skip graph redraw
     */
    void resize_history(const int points, bool redraw=true);

    QHBoxLayout *const m_layout;
    QGroupBox *const m_legend;
    QVBoxLayout *const m_legend_layout;
    QScrollArea *const m_button_area;
    QWidget *const m_scroll_container;
    QVBoxLayout *const m_scroll_layout;
    QPushButton *const m_add_button;
    QPushButton *const m_configure_button;
    QMenu *const m_main_menu;
    bool m_fixed_limits = false;

    double m_miny;
    double m_maxy;
};


#endif // TREND_WINDOW_H
