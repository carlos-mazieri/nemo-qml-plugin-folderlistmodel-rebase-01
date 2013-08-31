#include "dirmodel.h"
#include "tempfiles.h"
#include <stdio.h>

#include <QApplication>
#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QFileInfo>
#include <QDirIterator>
#include <QFile>


#define TIME_TO_PROCESS       2300
#define TIME_TO_REFRESH_DIR   90

#if QT_VERSION  >= 0x050000
#define  QSKIP_ALL_TESTS(statement)   QSKIP(statement)
#else
#define  QSKIP_ALL_TESTS(statement)   QSKIP(statement,SkipAll)
#endif

#if  QT_VERSION < 0x050000
# define QFileDevice   QFile
#endif


class TestDirModel : public QObject
{
   Q_OBJECT

public:
       TestDirModel();
      ~TestDirModel();

protected slots:
    void slotError(QString title, QString message);

private Q_SLOTS:
    void initTestCase();       //before all tests
    void cleanupTestCase();    //after all tests
    void init();               //before every test
    void cleanup();            //after every test

private Q_SLOTS: // test cases
    void removeSingleFile();
    void renameFileAndDir();
    void createNewDir();

private:
    void initDeepDirs();
    void cleanDeepDirs();
    void initModels();
    void cleanModels();
    bool compareDirectories(const QString& d1,
                            const QString& d2);

    bool createLink(const QString& fullSouce,
                    const QString& link,
                    bool  fullLink = false);


private:
    DeepDir   *    m_deepDir_01;
    DeepDir   *    m_deepDir_02;
    DeepDir   *    m_deepDir_03;

    DirModel  *    m_dirModel_01;
    DirModel  *    m_dirModel_02;

    QString        m_currentPath;
    QString        m_fileToRemoveInProgressSignal;
    bool           m_receivedErrorSignal;

};

TestDirModel::TestDirModel() : m_deepDir_01(0)
                                    ,m_deepDir_02(0)
                                    ,m_deepDir_03(0)
                                    ,m_dirModel_01(0)
                                    ,m_dirModel_02(0)
{
  
}


bool TestDirModel::createLink(const QString &fullSouce, const QString &link, bool fullLink)
{
    bool ret = false;

    QFileInfo source(fullSouce);
    if (source.exists())
    {
        if (fullLink)
        {
            QFileInfo lnk(link);
            ret = QFile::link(source.absoluteFilePath(), lnk.absoluteFilePath());
        }
        else
        {
            QString curDir = QDir::currentPath();
            if (QDir::setCurrent(source.absolutePath()))
            {
                QFileInfo lnk(link);
                if (source.absolutePath() != lnk.absolutePath())
                {
                    if (lnk.isAbsolute())
                    {
                        ret = QFile::link(source.absoluteFilePath(), link);
                    }
                    else
                    {
                        QDir relative(lnk.absolutePath());
                        if (relative.exists() || relative.mkpath(lnk.absolutePath()))
                        {
                           int diff=0;
                           QStringList sourceDirs = source.absolutePath().
                                                    split(QDir::separator(), QString::SkipEmptyParts);
                           QStringList targetDirs = lnk.absolutePath().
                                                    split(QDir::separator(), QString::SkipEmptyParts);
                           while (diff < sourceDirs.count())
                           {
                               if (sourceDirs.at(diff) !=
                                       targetDirs.at(diff))
                               {
                                   break;
                               }
                               diff++;
                           }
                           QString relativePath = sourceDirs.at(diff);
                           QString gap(QLatin1String("..") + QDir::separator());
                           while (diff++ < targetDirs.count())
                           {
                               relativePath.prepend(gap);
                           }
                           ret = QFile::link(relativePath
                                                 + QDir::separator()
                                                 + source.fileName(),
                                             link);
                        }
                    }
                }
                else
                {
                    ret = QFile::link(source.fileName(), link);
                }
                QDir::setCurrent(curDir);
            }
        }
    }
    return ret;
}

bool TestDirModel::compareDirectories(const QString &d1, const QString &d2)
{
    QDirIterator d1Info(d1,
                    QDir::Files | QDir::Hidden | QDir::System,
                    QDirIterator::Subdirectories);

    int len = d1.length();

    while (d1Info.hasNext() &&  !d1Info.next().isEmpty())
    {
        QString target(d2  + d1Info.fileInfo().absoluteFilePath().mid(len));

        QFileInfo d2Info(target);
        if (d1Info.fileName() != d2Info.fileName())
        {
            qDebug() << "false name" << d1Info.fileName() << d2Info.fileName();
            return false;
        }
        if (d1Info.fileInfo().size() != d2Info.size())
        {
            qDebug() << "false size" << d1Info.fileName() << d1Info.fileInfo().size()
                                    << d2Info.fileName() << d2Info.size();
            return false;
        }
        if (d1Info.fileInfo().permissions() != d2Info.permissions())
        {
            qDebug() << "false permissions" << d1Info.fileName() << d2Info.fileName();
            return false;
        }       
    }

    return true;
}


void TestDirModel::slotError(QString title, QString message)
{
    qWarning("Received Error: [title: %s] [message: %s]", qPrintable(title), qPrintable(message));
    m_receivedErrorSignal = true;
}

TestDirModel::~TestDirModel()
{

}

void TestDirModel::initDeepDirs()
{
    cleanDeepDirs();
}


void TestDirModel::cleanDeepDirs()
{
    if (m_deepDir_01) delete m_deepDir_01;
    if (m_deepDir_02) delete m_deepDir_02;
    if (m_deepDir_03) delete m_deepDir_03;
    m_deepDir_01 = 0;
    m_deepDir_02 = 0;
    m_deepDir_03 = 0;
}


void TestDirModel::initModels()
{
    cleanModels();
}


void TestDirModel::cleanModels()
{
    if (m_dirModel_01) delete m_dirModel_01;
    if (m_dirModel_02) delete m_dirModel_02;
    m_dirModel_01 = 0;
    m_dirModel_02 = 0;
}

void TestDirModel::initTestCase()
{
   qRegisterMetaType<QVector<QFileInfo> >("QVector<QFileInfo>");
   qRegisterMetaType<QFileInfo>("QFileInfo");
}


void TestDirModel::cleanupTestCase()
{
    cleanDeepDirs();
    cleanModels();
}


void TestDirModel::init()
{  
   initDeepDirs();
   initModels();
   m_receivedErrorSignal = false;  
}


void TestDirModel::cleanup()
{
    cleanDeepDirs();
    cleanModels();   
    m_receivedErrorSignal  = false;  
}



void TestDirModel::removeSingleFile()
{
    QString orig("removeSingleFile_orig");

    //it creates /tmp/removeSingleFile_orig
    //DeepDir destructor removes the directory and its content
    m_deepDir_01 = new DeepDir(orig, 0);
    QCOMPARE( QFileInfo(m_deepDir_01->path()).exists(),  true);

    //point m_dirModel_01 to /tmp/removeSingleFile_orig and make sure it is empty
    m_dirModel_01 = new DirModel();
    m_dirModel_01->setPath(m_deepDir_01->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->rowCount(), 0);

    //creates a file in /tmp/removeSingleFile_orig and make sure it was created
    TempFiles  tempFile;
    tempFile.addSubDirLevel(orig);
    tempFile.create(1);
    m_dirModel_01->refresh();
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->rowCount(), 1);

    //try to remove
    m_dirModel_01->rm(tempFile.createdList());
    QTest::qWait(TIME_TO_PROCESS);

    //check if it was removed
    m_dirModel_01->refresh();
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->rowCount(), 0);
}


void TestDirModel::renameFileAndDir()
{
    QString orig("renameSingleFile_orig");

    m_deepDir_01 = new DeepDir(orig, 0);
    QCOMPARE( QFileInfo(m_deepDir_01->path()).exists(),  true);

    QString dir("anotherDir");
    TempFiles  tempFile;
    tempFile.addSubDirLevel(orig);
    tempFile.create(1);
    tempFile.addSubDirLevel(dir);

    m_dirModel_01 = new DirModel();
    m_dirModel_01->setPath(m_deepDir_01->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->rowCount(), 2); //one file and one dir

    const int rowDir  = 0;
    const int rowFile = 1;
    QModelIndex dirIndex  = m_dirModel_01->index(rowDir,0);
    QModelIndex fileIndex = m_dirModel_01->index(rowFile,0);
    QCOMPARE(m_dirModel_01->data(dirIndex, DirModel::FileNameRole).toString(), dir);

    QString filename(m_dirModel_01->data(fileIndex, DirModel::FileNameRole).toString());

    QString expectedDirName("renamedDir");
    bool ret = m_dirModel_01->rename(rowDir, expectedDirName);
    QTest::qWait(TIME_TO_PROCESS);
    QCOMPARE(ret,    true);
    dirIndex  = m_dirModel_01->index(rowDir,0);
    QCOMPARE(m_dirModel_01->data(dirIndex, DirModel::FileNameRole).toString(), expectedDirName);
    QCOMPARE(m_dirModel_01->data(dirIndex, DirModel::IsDirRole).toBool(),   true);

    QString expectedFIleName("remanedFile");
    ret = m_dirModel_01->rename(rowFile, expectedFIleName);
    QTest::qWait(TIME_TO_PROCESS);
    QCOMPARE(ret,    true);
    fileIndex  = m_dirModel_01->index(rowFile,0);
    QVERIFY(filename != m_dirModel_01->data(fileIndex, DirModel::FileNameRole).toString());
    filename = m_dirModel_01->data(fileIndex, DirModel::FileNameRole).toString();
    QCOMPARE(filename,   expectedFIleName);

    QCOMPARE(m_receivedErrorSignal,   false); // no error signal received
}

void TestDirModel::createNewDir()
{
    QString orig("createNewDir_orig");

    m_deepDir_01 = new DeepDir(orig, 0);
    QCOMPARE( QFileInfo(m_deepDir_01->path()).exists(),  true);

    m_dirModel_01 = new DirModel();
    m_dirModel_01->setPath(m_deepDir_01->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->rowCount(), 0);

    QString newdir("newDir");
    m_dirModel_01->mkdir(newdir);
    QTest::qWait(TIME_TO_PROCESS);
    QCOMPARE(m_dirModel_01->rowCount(), 1);

    QModelIndex dirIndex  = m_dirModel_01->index(0,0);
    QCOMPARE(m_dirModel_01->data(dirIndex, DirModel::IsDirRole).toBool(),   true);
    QCOMPARE(m_dirModel_01->data(dirIndex, DirModel::FileNameRole).toString(), newdir);

    QCOMPARE(m_receivedErrorSignal,   false); // no error signal received
}


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    TestDirModel tc;
    int ret = QTest::qExec(&tc, argc, argv);

    return ret;
}


#include "tst_folderlistmodel.moc"
