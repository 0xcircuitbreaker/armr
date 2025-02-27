// Copyright (c) 2017-2018 The ARMR developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef GUIUTIL_H
#define GUIUTIL_H

#include <QString>
#include <QObject>
#include <QMessageBox>
#include <stdint.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>

QT_BEGIN_NAMESPACE
class QFont;
class QLineEdit;
class QWidget;
class QDateTime;
class QUrl;
class QAbstractItemView;
QT_END_NAMESPACE
class SendCoinsRecipient;

namespace fs = boost::filesystem;

/** Utility functions used by the Bitcoin Qt UI.
 */
namespace GUIUtil
{
// Create human-readable string from date
QString dateTimeStr(const QDateTime &datetime);
QString dateTimeStr(qint64 nTime);
QString formatDurationStr(int secs);
QString formatServicesStr(quint64 mask);

/* Format a CNodeCombinedStats.dPingTime into a user-readable string or display N/A, if 0*/
QString formatPingTime(double dPingTime);

/* Format a CNodeCombinedStats.nTimeOffset into a user-readable string. */
QString formatTimeOffset(int64_t nTimeOffset);
// Returns true if given address+amount meets "dust" definition
QString getEntryData(QAbstractItemView *view, int column, int role);
// Render Bitcoin addresses in monospace font
QFont bitcoinAddressFont();

/** Save window size and position */
void saveWindowGeometry(const QString &strSetting, QWidget *parent);
/** Restore window size and position */
void restoreWindowGeometry(const QString &strSetting, const QSize &defaultSizeIn, QWidget *parent);
/** Load global CSS style */
QString loadStyleSheet();

// Set up widgets for address and amounts
void setupAddressWidget(QLineEdit *widget, QWidget *parent);
void setupAmountWidget(QLineEdit *widget, QWidget *parent);

// Parse "ARMR:" URI into recipient object, return true on successful parsing
// See Bitcoin URI definition discussion here: https://bitcointalk.org/index.php?topic=33490.0
bool parseBitcoinURI(const QUrl &uri, SendCoinsRecipient *out);
bool parseBitcoinURI(QString uri, SendCoinsRecipient *out);

// HTML escaping for rich text controls
QString HtmlEscape(const QString &str, bool fMultiLine = false);
QString HtmlEscape(const std::string &str, bool fMultiLine = false);

/** Copy a field of the currently selected entry of a view to the clipboard. Does nothing if nothing
        is selected.
       @param[in] column  Data column to extract from the model
       @param[in] role    Data role to extract from the model
       @see  TransactionView::copyLabel, TransactionView::copyAmount, TransactionView::copyAddress
     */
void copyEntryData(QAbstractItemView *view, int column, int role = Qt::EditRole);

/** Return a field of the currently selected entry as a QString. Does nothing if nothing
        is selected.
       @param[in] column  Data column to extract from the model
       @see  TransactionView::copyLabel, TransactionView::copyAmount, TransactionView::copyAddress
     */
QList<QModelIndex> getEntryData(QAbstractItemView *view, int column);

void setClipboard(const QString& str);

/** Get save filename, mimics QFileDialog::getSaveFileName, except that it appends a default suffix
        when no suffix is provided by the user.

      @param[in] parent  Parent window (or 0)
      @param[in] caption Window caption (or empty, for default)
      @param[in] dir     Starting directory (or empty, to default to documents directory)
      @param[in] filter  Filter specification such as "Comma Separated Files (*.csv)"
      @param[out] selectedSuffixOut  Pointer to return the suffix (file type) that was selected (or 0).
                  Can be useful when choosing the save file format based on suffix.
     */

QString getSaveFileName(QWidget *parent = 0, const QString &caption = QString(),
                        const QString &dir = QString(), const QString &filter = QString(),
                        QString *selectedSuffixOut = 0);

/** Get connection type to call object slot in GUI thread with invokeMethod. The call will be blocking.

       @returns If called from the GUI thread, return a Qt::DirectConnection.
                If called from another thread, return a Qt::BlockingQueuedConnection.
    */
Qt::ConnectionType blockingGUIThreadConnection();

// Determine whether a widget is hidden behind other windows
bool isObscured(QWidget *w);

// Open debug.log
void openDebugLogfile();

// Open ARMR.conf
void openConfigfile();


/** Qt event filter that intercepts ToolTipChange events, and replaces the tooltip with a rich text
      representation if needed. This assures that Qt can word-wrap long tooltip messages.
      Tooltips longer than the provided size threshold (in characters) are wrapped.
     */
class ToolTipToRichTextFilter : public QObject
{
    Q_OBJECT

  public:
    explicit ToolTipToRichTextFilter(int size_threshold, QObject *parent = 0);

  protected:
    bool eventFilter(QObject *obj, QEvent *evt);

  private:
    int size_threshold;
};

bool GetStartOnSystemStartup();
bool SetStartOnSystemStartup(bool fAutoStart);

/* Convert QString to OS specific boost path through UTF-8 */
fs::path qstringToBoostPath(const QString &path);

/* Convert OS specific boost path to QString through UTF-8 */
QString boostPathToQString(const fs::path &path);

/** Help message for Bitcoin-Qt, shown with --help. */
class HelpMessageBox : public QMessageBox
{
    Q_OBJECT

  public:
    HelpMessageBox(QWidget *parent = 0);

    /** Show message box or print help message to standard output, based on operating system. */
    void showOrPrint();

    /** Print help message to console */
    void printToConsole();

  private:
    QString header;
    QString coreOptions;
    QString uiOptions;
};

} // namespace GUIUtil

#endif // GUIUTIL_H
