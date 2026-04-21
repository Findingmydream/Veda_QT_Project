#include "cameracapturedialog.h"

#include <QCamera>
#include <QCameraDevice>
#include <QMediaCaptureSession>
#include <QImageCapture>
#include <QVideoWidget>
#include <QMediaDevices>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QDir>
#include <QDateTime>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
#include <QPixmap>
#include <QIcon>

CameraCaptureDialog::CameraCaptureDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("웹캠 촬영");
    setModal(true);
    resize(580, 460);

    auto* root = new QVBoxLayout(this);
    root->setSpacing(10);

    // 상단: 실시간 영상 + 캡처 썸네일
    auto* topRow = new QHBoxLayout;

    videoWidget_ = new QVideoWidget;
    videoWidget_->setMinimumSize(380, 300);
    videoWidget_->setStyleSheet("background:#000;");
    topRow->addWidget(videoWidget_, 1);

    thumbBtn_ = new QPushButton;
    thumbBtn_->setFixedSize(120, 120);
    thumbBtn_->setIconSize(QSize(110, 110));
    thumbBtn_->setStyleSheet("background:#0d1117; border:1px solid #555; color:#888;");
    thumbBtn_->setText("(캡처\n미리보기)");
    thumbBtn_->setEnabled(false);
    thumbBtn_->setToolTip("클릭 → 저장된 파일 열기");
    topRow->addWidget(thumbBtn_, 0, Qt::AlignTop);

    root->addLayout(topRow, 1);

    auto* hint = new QLabel("📁  저장 위치: ./캡처본/");
    hint->setStyleSheet("color:#888; font-size:11px;");
    root->addWidget(hint);

    // 버튼 행
    auto* btnRow = new QHBoxLayout;
    captureBtn_ = new QPushButton("📸  캡처");
    saveBtn_    = new QPushButton("💾  저장");
    cancelBtn_  = new QPushButton("취소");
    saveBtn_->setEnabled(false);
    btnRow->addWidget(captureBtn_);
    btnRow->addStretch(1);
    btnRow->addWidget(saveBtn_);
    btnRow->addWidget(cancelBtn_);
    root->addLayout(btnRow);

    // 카메라 셋업
    QList<QCameraDevice> cams = QMediaDevices::videoInputs();
    if (cams.isEmpty()) {
        QMessageBox::warning(this, "알림", "사용 가능한 카메라를 찾을 수 없습니다.");
        QTimer::singleShot(0, this, &QDialog::reject);
        return;
    }

    camera_       = new QCamera(cams.first(), this);
    session_      = new QMediaCaptureSession(this);
    imageCapture_ = new QImageCapture(this);
    session_->setCamera(camera_);
    session_->setVideoOutput(videoWidget_);
    session_->setImageCapture(imageCapture_);

    connect(captureBtn_, &QPushButton::clicked, this, &CameraCaptureDialog::onCaptureClicked);
    connect(saveBtn_,    &QPushButton::clicked, this, &CameraCaptureDialog::onSaveClicked);
    connect(cancelBtn_,  &QPushButton::clicked, this, &QDialog::reject);
    connect(thumbBtn_,   &QPushButton::clicked, this, &CameraCaptureDialog::onThumbnailClicked);
    connect(imageCapture_, &QImageCapture::imageSaved,
            this, &CameraCaptureDialog::onImageSaved);

    camera_->start();
}

CameraCaptureDialog::~CameraCaptureDialog()
{
    if (camera_) camera_->stop();
}

void CameraCaptureDialog::onCaptureClicked()
{
    if (!imageCapture_ || !imageCapture_->isReadyForCapture()) {
        QMessageBox::information(this, "알림", "카메라가 아직 준비되지 않았습니다. 잠시 후 다시 시도하세요.");
        return;
    }
    QString dirPath = QDir::current().absoluteFilePath("캡처본");
    QDir().mkpath(dirPath);
    QString filename = QString("capture_%1.jpg")
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz"));
    QString full = dirPath + "/" + filename;
    imageCapture_->captureToFile(full);
}

void CameraCaptureDialog::onImageSaved(int /*id*/, const QString& path)
{
    lastSavedPath_ = path;
    QPixmap pix(path);
    if (!pix.isNull()) {
        thumbBtn_->setIcon(QIcon(pix.scaled(110, 110,
                                            Qt::KeepAspectRatio,
                                            Qt::SmoothTransformation)));
        thumbBtn_->setText("");
        thumbBtn_->setEnabled(true);
    }
    saveBtn_->setEnabled(true);
    flash();
}

void CameraCaptureDialog::flash()
{
    // 영상/썸네일 영역에 흰 오버레이를 잠깐 띄움
    auto makeFlash = [this](QWidget* target) {
        auto* f = new QLabel(this);
        f->setStyleSheet("background:rgba(255,255,255,200);");
        f->setAttribute(Qt::WA_TransparentForMouseEvents);
        QPoint pos = target->mapTo(this, QPoint(0, 0));
        f->setGeometry(pos.x(), pos.y(), target->width(), target->height());
        f->raise();
        f->show();
        QTimer::singleShot(170, f, [f]{ f->deleteLater(); });
    };
    makeFlash(videoWidget_);
    makeFlash(thumbBtn_);
}

void CameraCaptureDialog::onThumbnailClicked()
{
    if (lastSavedPath_.isEmpty()) return;
    QDesktopServices::openUrl(QUrl::fromLocalFile(lastSavedPath_));
}

void CameraCaptureDialog::onSaveClicked()
{
    if (lastSavedPath_.isEmpty()) {
        QMessageBox::information(this, "알림", "먼저 [캡처] 버튼으로 사진을 찍으세요.");
        return;
    }
    accept();
}
