#include "al_quran.h"

#include <QLabel>
#include <QAction>
#include <QGridLayout>
#include <QListWidget>

#include "core/controls/quran_reciter.h"
#include "core/controls/quran_reader.h"

_INITIALIZE_EASYLOGGINGPP

const char* AlQuran::kAuthor       = "Project Islam Authors";
const char* AlQuran::kName         = "Al-Quran";
const char* AlQuran::kTitle        = "Al-Qur'an";
const char* AlQuran::kDescription  = "Al-Qur'an recitation, reading and help memorize yourself.";

AlQuran::AlQuran()
{
    memory::turnToNullPtr(m_reciter, m_reader, m_bookmarkBar);
    setExtensionInfo(ExtensionInfo(kMajorVersion, kMinorVersion, QString(kAuthor), 
                                   QString(kName), QString(kTitle), QString(kDescription)));
}

AlQuran::~AlQuran()
{
}

bool AlQuran::initialize()
{
    if (!ExtensionBase::initialize()) {
        // Do not proceed
        return false;
    }
    
    // Do not trace location before calling parent's initialize
    _TRACE;
    memory::deleteAll(m_reciter, m_reader, m_bookmarkBar);
    m_reciter = new QuranReciter(extension()->dataHolder()->quranArabic(), extension()->container());
    m_reader = new QuranReader(extension()->dataHolder()->quranArabic(), 
                               extension()->dataHolder()->quranTranslation(), extension()->dataHolder()->quranTransliteration(), 
                               extension()->container());    
    m_reciter->hideChapterSelector();
    m_reciter->hideVerseRangeSelector();
    m_reciter->hideCurrentVerseSelector();
    QObject::connect(m_reciter, SIGNAL(chapterChanged(const quran::Chapter*)), this, SLOT(onChapterChangedReciter(const quran::Chapter*)));
    QObject::connect(m_reciter, SIGNAL(verseRangeChanged(int,int)), this, SLOT(onVerseRangeChangedReciter(int,int)));
    QObject::connect(m_reader, SIGNAL(chapterChanged(const quran::Chapter*)), this, SLOT(onChapterChangedReader(const quran::Chapter*)));
    QObject::connect(m_reader, SIGNAL(verseRangeChanged(int,int)), this, SLOT(onVerseRangeChangedReader(int,int)));
    QObject::connect(m_reader, SIGNAL(currentVerseChanged(int)), this, SLOT(onSelectedVerseChangedReader(int)));
    QObject::connect(m_reciter, SIGNAL(currentVerseChanged(int)), this, SLOT(onSelectedVerseChangedReciter(int)));
    
    // Bookmarks bar
    m_bookmarkBar = new BookmarksBar(settingsKeyPrefix(), extension()->container());
    QObject::connect(m_bookmarkBar, SIGNAL(selectionChanged(Bookmark*)), this, SLOT(onBookmarkChanged(Bookmark*)));
    
    initializeMenu();
    
    // Force trigger this slot
    onContainerGeometryChanged(extension()->container()->width(), extension()->container()->height());
    return true;
}

void AlQuran::initializeMenu()
{
    _TRACE;
    QAction* showReaderAction = extensionMenu()->addAction("Show Reader");
    QObject::connect(showReaderAction, SIGNAL(toggled(bool)), this, SLOT(toggleReader(bool)));
    showReaderAction->setCheckable(true);
    showReaderAction->setChecked(
                toggleReader(setting("show_reader", QVariant(true)).toBool()));
    
    QAction* showReciterAction = extensionMenu()->addAction("Show Reciter");
    QObject::connect(showReciterAction, SIGNAL(toggled(bool)), this, SLOT(toggleReciter(bool)));
    showReciterAction->setCheckable(true);
    showReciterAction->setChecked(
                toggleReciter(setting("show_reciter", QVariant(true)).toBool()));
    
    QAction* showBookmarksAction = extensionMenu()->addAction("Bookmarks");
    QObject::connect(showBookmarksAction, SIGNAL(toggled(bool)), this, SLOT(toggleBookmarkBar(bool)));
    showBookmarksAction->setCheckable(true);
    showBookmarksAction->setChecked(
                toggleBookmarkBar(setting("show_bookmarks", QVariant(true)).toBool()));
}

void AlQuran::onContainerGeometryChanged(int w, int h)
{
    const int kCenterX = (w / 2) - (m_reciter->width() / 2);
    const int kBottom = h - m_reciter->height();
    m_reciter->move(kCenterX, kBottom);
    
    if (m_bookmarkBar->isVisible()) {
        m_bookmarkBar->resize(BookmarksBar::kBookmarkBarWidth, h - m_reciter->height());
        m_bookmarkBar->move(w - BookmarksBar::kBookmarkBarWidth, 0);
    }
    
    m_reader->resize(w - (m_bookmarkBar->isVisible() ? BookmarksBar::kBookmarkBarWidth : 0), 
                     h - m_reciter->height());
}

void AlQuran::onActivated()
{
}

void AlQuran::onDeactivated()
{
    // Stop if reciting
    if (m_reciter != nullptr) {
        m_reciter->stopIfPlaying();
    }
}

void AlQuran::onChapterChangedReciter(const quran::Chapter* chapter)
{
    if (m_reader != nullptr) {
        m_reader->changeChapter(chapter->name());
    }
}

void AlQuran::onVerseRangeChangedReciter(int from, int to)
{
    if (m_reader != nullptr) {
        m_reader->changeVerseRange(from, to);
    }
}

void AlQuran::onChapterChangedReader(const quran::Chapter* chapter)
{
    if (m_reciter != nullptr) {
        m_reciter->changeChapter(chapter->name());
    }
}

void AlQuran::onVerseRangeChangedReader(int from, int to)
{
    if (m_reciter != nullptr) {
        m_reciter->changeVerseRange(from, to);
    }
}

void AlQuran::onSelectedVerseChangedReciter(int verseNumber)
{
    if (m_reader != nullptr) {
        if (m_reader->currentVerseNumber() != verseNumber) {
            m_reader->highlightVerse(verseNumber);
        }
    }
}

void AlQuran::onSelectedVerseChangedReader(int index)
{
    if (m_reciter != nullptr) {
        m_reciter->changeVerse(index);
    }
}

void AlQuran::onBookmarkChanged(Bookmark* bookmark)
{
    m_reader->changeChapter(static_cast<quran::Chapter::Name>(bookmark->chapter()));
    m_reader->changeVerseRange(bookmark->verseFrom(), bookmark->verseTo());
}

bool AlQuran::toggleReciter(bool val)
{
    _TRACE;
    m_reciter->setVisible(val);
    saveSetting("show_reciter", QVariant(val));
    return val;
}

bool AlQuran::toggleReader(bool val)
{
    _TRACE;
    m_reader->setVisible(val);
    saveSetting("show_reader", QVariant(val));
    if (!val) {
        m_reciter->showChapterSelector();
        m_reciter->showVerseRangeSelector();
        m_reciter->showCurrentVerseSelector();
    } else {
        m_reciter->hideChapterSelector();
        m_reciter->hideVerseRangeSelector();
        m_reciter->hideCurrentVerseSelector();
    }
    return val;
}

bool AlQuran::toggleBookmarkBar(bool val)
{
    _TRACE;
    m_bookmarkBar->setVisible(val);
    onContainerGeometryChanged(extension()->containerWidth(), extension()->containerHeight());
    saveSetting("show_bookmarks", QVariant(val));
    return val;
}
