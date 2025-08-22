// mainwindow.cpp - Version 8.4 (Sửa lỗi tua video)
#include "mainwindow.h"
#include "playerpanel.h"
#include "sidepanel.h" 
#include "exportpanel.h" 
#include "librarywidget.h" 
#include "videoworker.h"
#include "videowidget.h"

#include <QSplitter>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QKeyEvent>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QAudioSink>
#include <QMediaDevices>
#include <QThread>
#include <QStandardPaths>
#include <QUuid>
#include <QSettings>
#include <QCloseEvent>
#include <QPainter>
#include <QCursor> 
#include <QLineEdit> 
#include <QSpinBox>  
#include <QtConcurrent>
#include <QThreadPool> 

Q_DECLARE_METATYPE(VideoProcessor::AudioParams)
Q_DECLARE_METATYPE(AVRational)
Q_DECLARE_METATYPE(QListWidgetItem*)

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    qRegisterMetaType<VideoProcessor::AudioParams>();
    qRegisterMetaType<AVRational>();
    qRegisterMetaType<QListWidgetItem*>();

    setupUi();
    setupVideoWorker();
    setupTempDirectory();
    loadSettings();
    setAcceptDrops(true);

    emit playerStateChanged(false);
}

MainWindow::~MainWindow()
{
    emit requestStop();
    m_videoThread->quit();
    m_videoThread->wait();

    cleanupAudio();
    cleanupTempDirectory();
}

void MainWindow::setupUi()
{
    this->setWindowTitle("Frame Capture v6.0 (Stable)");
    this->setWindowIcon(QIcon(":/icons/icon.png"));
    this->resize(1600, 900); 

    mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->setHandleWidth(1);
    mainSplitter->setStyleSheet("QSplitter::handle { background-color: #606060; }");
    this->setCentralWidget(mainSplitter);

    m_playerPanel = new PlayerPanel();
    m_sidePanel = new SidePanel();

    mainSplitter->addWidget(m_playerPanel);
    mainSplitter->addWidget(m_sidePanel);

    // --- Connections ---
    connect(m_playerPanel, &PlayerPanel::openFileClicked, this, &MainWindow::onOpenFile);
    connect(m_playerPanel, &PlayerPanel::playPauseClicked, this, &MainWindow::onPlayPause);
    connect(m_playerPanel, &PlayerPanel::nextFrameClicked, this, [this](){ emit requestNextFrame(); });
    connect(m_playerPanel, &PlayerPanel::prevFrameClicked, this, [this](){ emit requestPrevFrame(); });
    connect(m_playerPanel, &PlayerPanel::captureClicked, this, &MainWindow::onCapture);
    connect(m_playerPanel, &PlayerPanel::captureAndExportClicked, this, &MainWindow::onCaptureAndExport);
    connect(m_playerPanel, &PlayerPanel::toggleRightPanelClicked, this, &MainWindow::onToggleRightPanel);
    connect(m_playerPanel, &PlayerPanel::timelinePressed, this, [this](){ m_isScrubbing = true; });
    connect(m_playerPanel, &PlayerPanel::timelineReleased, this, &MainWindow::onTimelineReleased);
    connect(m_playerPanel, &PlayerPanel::timelineMoved, this, &MainWindow::onTimelineMoved);
    connect(m_playerPanel, &PlayerPanel::muteClicked, this, &MainWindow::onMuteClicked);
    connect(m_playerPanel, &PlayerPanel::volumeChanged, this, &MainWindow::onVolumeChanged);

    connect(m_sidePanel, &SidePanel::exportImageRequested, this, &MainWindow::onExportImage);
    connect(m_sidePanel, &SidePanel::addImagesToLibraryRequested, this, &MainWindow::onAddImagesToLibrary);
    connect(m_sidePanel, &SidePanel::newImagesDropped, this, &MainWindow::onImagesDroppedOnLibrary);
    connect(m_sidePanel, &SidePanel::fileDeleted, this, [this](const QString& filePath){
        m_capturedFramePaths.removeOne(filePath);
    });

    connect(this, &MainWindow::playerStateChanged, m_playerPanel, &PlayerPanel::updatePlayerState);
    connect(this, &MainWindow::newFrameReady, this, [this](const FrameData& frameData, qint64 duration, double frameRate, const AVRational& timeBase){
        if (!m_isScrubbing) {
            m_playerPanel->updateUIWithFrame(frameData, duration, frameRate, timeBase);
        }
    });
}

void MainWindow::setupVideoWorker()
{
    m_videoThread = std::make_unique<QThread>();
    m_videoWorker = std::make_unique<VideoWorker>();
    m_videoWorker->moveToThread(m_videoThread.get());

    connect(this, &MainWindow::requestOpenFile, m_videoWorker.get(), &VideoWorker::processOpenFile);
    connect(this, &MainWindow::requestSeek, m_videoWorker.get(), &VideoWorker::processSeek);
    connect(this, &MainWindow::requestPlayPause, m_videoWorker.get(), &VideoWorker::processPlayPause);
    connect(this, &MainWindow::requestNextFrame, m_videoWorker.get(), &VideoWorker::processNextFrame);
    connect(this, &MainWindow::requestPrevFrame, m_videoWorker.get(), &VideoWorker::processPrevFrame);
    connect(this, &MainWindow::requestStop, m_videoWorker.get(), &VideoWorker::stop);

    connect(m_videoWorker.get(), &VideoWorker::fileOpened, this, &MainWindow::onFileOpened);
    connect(m_videoWorker.get(), &VideoWorker::frameReady, this, &MainWindow::onFrameReady);
    
    m_videoThread->start();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("splitterState", mainSplitter->saveState());
    if (!m_currentVideoPath.isEmpty()) {
        settings.setValue("lastUsedDir", QFileInfo(m_currentVideoPath).absolutePath());
    }
}

void MainWindow::loadSettings()
{
    QSettings settings;
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    
    QByteArray splitterState = settings.value("splitterState").toByteArray();
    if (!splitterState.isEmpty()) {
        mainSplitter->restoreState(splitterState);
    } else {
        QTimer::singleShot(0, this, [this]{
            mainSplitter->setSizes({width() * 2 / 3, width() / 3});
        });
    }
    m_lastUsedDir = settings.value("lastUsedDir", QDir::homePath()).toString();
}

void MainWindow::setupTempDirectory()
{
    m_tempPath = QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation)).filePath("FrameCapture_" + QUuid::createUuid().toString());
    QDir dir(m_tempPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

void MainWindow::cleanupTempDirectory()
{
    QDir dir(m_tempPath);
    if (dir.exists()) {
        dir.removeRecursively();
    }
}

void MainWindow::onFileOpened(bool success, VideoProcessor::AudioParams params, double frameRate, qint64 duration, AVRational timeBase)
{
    emit playerStateChanged(success);

    if (success) {
        m_frameRate = frameRate;
        m_duration = duration;
        m_timeBase = timeBase;
        
        cleanupAudio();
        if(params.isValid) {
            QAudioFormat format;
            format.setSampleRate(params.sample_rate);
            format.setChannelCount(params.channels);
            format.setSampleFormat(QAudioFormat::Int16);
            
            const auto& devices = QMediaDevices::defaultAudioOutput();
            if(!devices.isDefault()) {
                 QMessageBox::warning(this, "Lỗi Âm thanh", "Không tìm thấy thiết bị âm thanh mặc định.");
            } else {
                m_audioSink = new QAudioSink(devices, format);
                int bufferSize = params.sample_rate * params.channels * (16 / 8) * 2.0; 
                m_audioSink->setBufferSize(bufferSize);
                m_audioDevice = m_audioSink->start();
                onVolumeChanged(100); 
            }
        }
    } else {
        QMessageBox::warning(this, "Lỗi", "Không thể mở file video: " + m_currentVideoPath);
    }
}

void MainWindow::onFrameReady(const FrameData &frameData)
{
    emit newFrameReady(frameData, m_duration, m_frameRate, m_timeBase);
    if(m_audioDevice && !frameData.audioData.isEmpty()) {
        m_audioDevice->write(frameData.audioData);
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (!event->mimeData()->hasUrls()) {
        event->ignore();
        return;
    }
    const QUrl url = event->mimeData()->urls().first();
    const QFileInfo fileInfo(url.toLocalFile());
    const QString suffix = fileInfo.suffix().toLower();
    const bool isVideo = (suffix == "mp4" || suffix == "avi" || suffix == "mkv" || suffix == "mov");

    if (isVideo) {
        if (!m_sidePanel->getLibraryWidget()->rect().contains(m_sidePanel->getLibraryWidget()->mapFromGlobal(QCursor::pos()))) {
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
    } else {
        event->ignore();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
        QString filePath = urls.first().toLocalFile();
        openVideoFile(filePath);
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (qobject_cast<QLineEdit*>(focusWidget()) || qobject_cast<QSpinBox*>(focusWidget())) {
        QMainWindow::keyPressEvent(event);
        return;
    }
    switch (event->key()) {
    case Qt::Key_Space: 
        onPlayPause(); 
        event->accept();
        break;
    case Qt::Key_Right: 
        emit requestNextFrame();
        event->accept();
        break;
    case Qt::Key_Left: 
        emit requestPrevFrame();
        event->accept();
        break;
    default: 
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::openVideoFile(const QString& filePath)
{
    cleanupTempDirectory();
    setupTempDirectory();

    m_currentVideoPath = filePath;
    m_sidePanel->getExportPanel()->setSavePath(QFileInfo(filePath).absolutePath());
    m_sidePanel->getLibraryWidget()->clear();
    m_capturedFramePaths.clear();
    m_sidePanel->getViewPanel()->setImages({});
    emit requestOpenFile(filePath);
}

void MainWindow::onOpenFile()
{
    QString path = QFileDialog::getOpenFileName(this, "Mở file video", m_lastUsedDir, "Video Files (*.mp4 *.avi *.mkv *.mov)");
    if (!path.isEmpty()) {
        openVideoFile(path);
    }
    this->setFocus();
}

void MainWindow::onPlayPause()
{
    m_isPlaying = !m_isPlaying;
    m_playerPanel->setPlayPauseButtonIcon(m_isPlaying);
    emit requestPlayPause(m_isPlaying);
    this->setFocus();
}

void MainWindow::onCapture()
{
    QImage currentFrame = m_playerPanel->getVideoWidget()->getCurrentImage();
    if (!currentFrame.isNull()) {
        QThreadPool::globalInstance()->start([this, currentFrame]() {
            QString fileName = QUuid::createUuid().toString() + ".png";
            QString filePath = QDir(m_tempPath).filePath(fileName);
            if (currentFrame.save(filePath, "PNG")) {
                QMetaObject::invokeMethod(this, "addImageToList", Qt::QueuedConnection,
                                          Q_ARG(QString, filePath));
            }
        });
    }
    this->setFocus();
}

void MainWindow::onCaptureAndExport()
{
    QImage currentFrame = m_playerPanel->getVideoWidget()->getCurrentImage();
    if (!currentFrame.isNull()) {
        onExportImage(currentFrame);
    }
    this->setFocus();
}

void MainWindow::onMuteClicked()
{
}

void MainWindow::onVolumeChanged(int volume)
{
    if (m_audioSink) {
        m_audioSink->setVolume(volume / 100.0f);
    }
    if (volume > 0) {
        m_lastVolume = volume / 100.0f;
    }
}

void MainWindow::onToggleRightPanel()
{
    QList<int> sizes = mainSplitter->sizes();
    if (sizes.count() == 2) {
        bool isHidden = mainSplitter->widget(1)->width() == 0;
        if (isHidden) {
             mainSplitter->setSizes({width() * 2 / 3, width() / 3});
        } else {
             mainSplitter->setSizes({sizes[0] + sizes[1], 0});
        }
    }
}

void MainWindow::onTimelineMoved(int position)
{
    if (m_duration > 0) {
        qint64 time = m_duration * (double)position / 1000.0;
        m_playerPanel->updateTimeLabelOnly(time, m_duration, m_frameRate);
        // Chỉ gửi yêu cầu seek khi người dùng đang kéo thả
        if (m_isScrubbing) {
            emit requestSeek(time);
        }
    }
}

void MainWindow::onTimelineReleased()
{
    m_isScrubbing = false;
    if (m_isPlaying) {
        emit requestPlayPause(true);
    }
    this->setFocus();
}

void MainWindow::onExportImage(const QImage& image) 
{
    if (image.isNull()) {
        QMessageBox::warning(this, "Lỗi", "Không có ảnh để xuất.");
        return;
    }
    ExportPanel* exportPanel = m_sidePanel->getExportPanel();
    QString savePath = exportPanel->getSavePath();
    if (savePath.isEmpty()) {
        savePath = QFileDialog::getExistingDirectory(this, "Chọn thư mục lưu");
        if(savePath.isEmpty()) return;
        exportPanel->setSavePath(savePath);
    }
    
    QString format = exportPanel->getSelectedFormat().toLower();
    QString baseName = QFileInfo(m_currentVideoPath).baseName();
    if (baseName.isEmpty()) {
        baseName = "capture";
    }
    QString fullPath = generateUniqueFilename(baseName, format);

    if (image.save(fullPath)) {
        QMessageBox::information(this, "Thành công", "Đã lưu ảnh tại:\n" + fullPath);
    } else {
        QMessageBox::critical(this, "Lỗi", "Không thể lưu ảnh.");
    }
}

void MainWindow::onAddImagesToLibrary()
{
    QStringList filePaths = QFileDialog::getOpenFileNames(
        this, "Thêm ảnh vào thư viện", m_lastUsedDir, "Image Files (*.png *.jpg *.jpeg *.bmp)");
    for (const QString &path : filePaths) {
        QString newFileName = QUuid::createUuid().toString() + "." + QFileInfo(path).suffix();
        QString newPath = QDir(m_tempPath).filePath(newFileName);
        if (QFile::copy(path, newPath)) {
            addImageToList(newPath);
        }
    }
}

void MainWindow::onImagesDroppedOnLibrary(const QList<QUrl> &urls)
{
    for (const QUrl &url : urls) {
        QString path = url.toLocalFile();
        QString newFileName = QUuid::createUuid().toString() + "." + QFileInfo(path).suffix();
        QString newPath = QDir(m_tempPath).filePath(newFileName);
        if (QFile::copy(path, newPath)) {
            addImageToList(newPath);
        }
    }
}

void MainWindow::cleanupAudio()
{
    if(m_audioSink) {
        m_audioSink->stop();
        delete m_audioSink;
        m_audioSink = nullptr;
        m_audioDevice = nullptr;
    }
}

QString MainWindow::generateUniqueFilename(const QString& baseName, const QString& extension)
{
    QString savePath = m_sidePanel->getExportPanel()->getSavePath();
    QString fullPath = QDir(savePath).filePath(baseName + "." + extension);
    int counter = 1;
    while (QFile::exists(fullPath)) {
        fullPath = QDir(savePath).filePath(QString("%1_%2.%3").arg(baseName).arg(counter).arg(extension));
        counter++;
    }
    return fullPath;
}

void MainWindow::ensureRightPanelVisible()
{
    if (mainSplitter->widget(1)->width() == 0) {
        onToggleRightPanel();
    }
}

void MainWindow::addImageToList(const QString &imagePath)
{
    ensureRightPanelVisible();
    QImage image(imagePath);
    if (image.isNull()) return;
    m_capturedFramePaths.append(imagePath);
    
    LibraryWidget* libraryWidget = m_sidePanel->getLibraryWidget();
    QPixmap thumbnailPixmap = QPixmap::fromImage(image.scaled(libraryWidget->iconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    
    QListWidgetItem *item = new QListWidgetItem(QIcon(thumbnailPixmap), "");
    item->setData(Qt::UserRole, imagePath); 
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Unchecked);
    libraryWidget->addItem(item);
}
