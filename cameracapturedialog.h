#pragma once
#include <QDialog>

class QCamera;
class QMediaCaptureSession;
class QImageCapture;
class QVideoWidget;
class QPushButton;
class QLabel;

class CameraCaptureDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CameraCaptureDialog(QWidget* parent = nullptr);
    ~CameraCaptureDialog();

    QString savedPath() const { return lastSavedPath_; }

private slots:
    void onCaptureClicked();
    void onImageSaved(int id, const QString& path);
    void onThumbnailClicked();
    void onSaveClicked();

private:
    void flash();

    QCamera*              camera_       = nullptr;
    QMediaCaptureSession* session_      = nullptr;
    QImageCapture*        imageCapture_ = nullptr;
    QVideoWidget*         videoWidget_  = nullptr;
    QPushButton*          captureBtn_   = nullptr;
    QPushButton*          saveBtn_      = nullptr;
    QPushButton*          cancelBtn_    = nullptr;
    QPushButton*          thumbBtn_     = nullptr;
    QString               lastSavedPath_;
};
