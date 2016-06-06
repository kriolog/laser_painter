#include "camera_settings.h"

#include <QTimer>
#include <QComboBox>
#include <QLabel>
#include <QCamera>
#include <QCameraImageCapture>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "video_frame_grabber.h"

Q_DECLARE_METATYPE(QCameraInfo)

namespace laser_painter {

CameraSettings::CameraSettings(
    VideoFrameGrabber* video_frame_grabber,
    QWidget* parent, Qt::WindowFlags f
)
    : QWidget(parent, f),
    _camera(0),
    _camera_image_capture(0),
    _camera_info()
{
    Q_ASSERT(video_frame_grabber);

    connect(this, &CameraSettings::cameraChanged, video_frame_grabber, &VideoFrameGrabber::installCamera);

    // HACK: regularly update list of avalilable cameras.
    // Instead, can use a 3rdparty library https://github.com/wang-bin/qdevicewatcher
    // to listen usb devices (Qt doesn't have a native support).
    QTimer* cameras_update_timer = new QTimer(this);
    cameras_update_timer->setInterval(1000);
    cameras_update_timer->start();
    connect(cameras_update_timer, SIGNAL(timeout()), this, SLOT(updateAvailableCameras()));

    _camera_cb = new QComboBox();
    connect(_camera_cb, SIGNAL(activated(int)), this, SLOT(changeCamera()));
    QLabel* camera_lb = new QLabel(tr("Camera"));
    camera_lb->setBuddy(_camera_cb);

    _resolution_cb = new QComboBox();
    connect(_resolution_cb, SIGNAL(activated(int)), this, SLOT(changeResolution()));
    QLabel* resolution_lb = new QLabel(tr("Resolution"));
    resolution_lb->setBuddy(_resolution_cb);

    updateAvailableCameras(true);

    QHBoxLayout* camera_lo = new QHBoxLayout();
    camera_lo->addWidget(camera_lb);
    camera_lo->addWidget(_camera_cb);
    camera_lo->addStretch();

    QHBoxLayout* resolution_lo = new QHBoxLayout();
    resolution_lo ->addWidget(resolution_lb);
    resolution_lo ->addWidget(_resolution_cb);
    resolution_lo ->addStretch();

    QVBoxLayout* main_lo = new QVBoxLayout();
    setLayout(main_lo);
    main_lo->addLayout(camera_lo);
    main_lo->addLayout(resolution_lo);
}

void CameraSettings::updateAvailableCameras(bool try_set_camera)
{
    QString cur_text =  _camera_cb->currentText();
    _camera_cb->clear();

    QCameraInfo default_camera_info = QCameraInfo::defaultCamera();
    bool has_default_camera = !default_camera_info.isNull();

    int default_camera_id = -1;
    foreach(const QCameraInfo& camera_info, QCameraInfo::availableCameras()) {
        _camera_cb->addItem(camera_info.description(), QVariant::fromValue(camera_info));
        if(try_set_camera && has_default_camera && camera_info == default_camera_info)
            default_camera_id = _camera_cb->count() - 1;
    }

    if(try_set_camera && has_default_camera && default_camera_id >= 0) {
        _camera_cb->setCurrentIndex(default_camera_id);
        changeCamera();
    } else if(try_set_camera && _camera_cb->count() > 0)
        _camera_cb->setCurrentIndex(0);
    else
        _camera_cb->setCurrentText(cur_text);
}

void CameraSettings::updateSupportedResolutions()
{
    Q_ASSERT(_camera_image_capture);

    QString cur_text =  _resolution_cb->currentText();
    _resolution_cb->clear();

    foreach(const QSize& resolution, _camera_image_capture->supportedResolutions())
        _resolution_cb->addItem(
            QString("%1x%2").arg(resolution.width()).arg(resolution.height()),
            QVariant::fromValue(resolution)
        );

    if(_resolution_cb->count() > 0)
        _resolution_cb->setCurrentIndex(_resolution_cb->count() - 1);
    else
        _resolution_cb->setCurrentText(cur_text);
}


void CameraSettings::changeCamera()
{
    QCameraInfo new_camera_info = _camera_cb->currentData().value<QCameraInfo>();
    if(_camera && _camera->state() == QCamera::ActiveState && _camera_info == new_camera_info)
        // Skip a try to choose the same camera which is in a working state.
        return;

    delete _camera_image_capture;
    delete _camera;

    _camera_info = _camera_cb->currentData().value<QCameraInfo>();
    _camera = new QCamera(_camera_info);
    _camera_image_capture = new QCameraImageCapture(_camera);

    emit cameraChanged(_camera);

    updateSupportedResolutions();
    changeResolution();

}

void CameraSettings::changeResolution()
{
    Q_ASSERT(_camera_image_capture);

    QSize resolution = _resolution_cb->currentData().value<QSize>();
    if(resolution.isEmpty())
        return;
    if(resolution == _camera_image_capture->encodingSettings().resolution())
        // Skip a try to choose the same resolution.
        return

    // Resolution is really supported by camera
    Q_ASSERT( qFind(_camera_image_capture->supportedResolutions(), resolution)
        != _camera_image_capture->supportedResolutions().end());

    QImageEncoderSettings image_settings;
    image_settings.setResolution(resolution);
    _camera_image_capture->setEncodingSettings(image_settings);

    emit resolutionChanged(resolution);
}

} // namespace laser_detector