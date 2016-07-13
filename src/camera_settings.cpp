#include "camera_settings.h"

#include <QSettings>
#include <QTimer>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QCamera>
#include <QCameraImageCapture>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "video_frame_grabber.h"

Q_DECLARE_METATYPE(QCameraInfo)

namespace laser_painter {

const QList<QSize> CameraSettings::_camera_common_resolutions = QList<QSize>()
    << QSize(160, 120)
    << QSize(176, 144)
    << QSize(320, 176)
    << QSize(320, 240)
    << QSize(352, 288)
    << QSize(432, 240)
    << QSize(544, 288)
    << QSize(640, 360)
    << QSize(640, 480)
    << QSize(752, 416)
    << QSize(800, 448)
    << QSize(800, 600)
    << QSize(864, 480)
    << QSize(960, 544)
    << QSize(960, 720)
    << QSize(1024, 576)
    << QSize(1024, 768)
    << QSize(1184, 656)
    << QSize(1280, 720)
    << QSize(1280, 800)
    << QSize(1280, 960)
    << QSize(1392, 768)
    << QSize(1504, 832)
    << QSize(1600, 896)
    << QSize(1600, 1200)
    << QSize(1712, 960)
    << QSize(1792, 1008)
    << QSize(1920, 1080);

CameraSettings::CameraSettings(
    VideoFrameGrabber* video_frame_grabber,
    QWidget* parent
)
    : QGroupBox(parent),
    _camera(0),
    _camera_image_capture(0),
    _camera_info()
{
    Q_ASSERT(video_frame_grabber);

    setTitle(tr("Camera Settings"));

    QSettings settings;

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
    QLabel* camera_lb = new QLabel(tr("Camera:"));
    camera_lb->setBuddy(_camera_cb);

    _resolution_cb = new QComboBox();
    connect(_resolution_cb, SIGNAL(activated(int)), this, SLOT(changeResolution()));
    QLabel* resolution_lb = new QLabel(tr("Resolution:"));
    resolution_lb->setBuddy(_resolution_cb);

    _flip_x_cb = new QCheckBox();
    connect(_flip_x_cb, &QCheckBox::toggled, video_frame_grabber, &VideoFrameGrabber::setFlipX);
    _flip_x_cb->setChecked(settings.value("CameraSettings/flip_x", true).toBool());
    QLabel* flip_x_lb = new QLabel(tr("Flip X:"));
    flip_x_lb->setToolTip(tr("Flip horisontally the camera image"));
    flip_x_lb->setBuddy(_flip_x_cb);

    _flip_y_cb = new QCheckBox();
    connect(_flip_y_cb, &QCheckBox::toggled, video_frame_grabber, &VideoFrameGrabber::setFlipY);
    _flip_y_cb->setChecked(settings.value("CameraSettings/flip_y", false).toBool());
    QLabel* flip_y_lb = new QLabel(tr("Flip Y:"));
    flip_y_lb->setToolTip(tr("Flip vertically the camera image"));
    flip_y_lb->setBuddy(_flip_y_cb);

    updateAvailableCameras(true);

    QHBoxLayout* camera_lo = new QHBoxLayout();
    camera_lo->addStretch();
    camera_lo->addWidget(camera_lb);
    camera_lo->addWidget(_camera_cb);

    QHBoxLayout* resolution_lo = new QHBoxLayout();
    resolution_lo->addStretch();
    resolution_lo->addWidget(resolution_lb);
    resolution_lo->addWidget(_resolution_cb);

    QHBoxLayout* flip_lo = new QHBoxLayout();
    flip_lo->addStretch();
    flip_lo->addWidget(flip_x_lb);
    flip_lo->addWidget(_flip_x_cb);
    flip_lo->addWidget(flip_y_lb);
    flip_lo->addWidget(_flip_y_cb);

    QVBoxLayout* main_lo = new QVBoxLayout();
    setLayout(main_lo);
    main_lo->addLayout(camera_lo);
    main_lo->addLayout(resolution_lo);
    main_lo->addLayout(flip_lo);
}

void CameraSettings::writeSettings() const
{
    QSettings settings;

    settings.beginGroup("CameraSettings");

    settings.setValue("flip_x", _flip_x_cb->isChecked());
    settings.setValue("flip_y", _flip_y_cb->isChecked());

    settings.endGroup();
}

QSize CameraSettings::currentResolution() const
{
    if(!_camera_image_capture)
        return QSize();
    return _camera_image_capture->encodingSettings().resolution();
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
    else {
    // Set list of common resolutions, camera is not guaranted to work
    // with a particular resolution from the list.
         foreach(const QSize& resolution, _camera_common_resolutions)
            _resolution_cb->addItem(
                QString("%1x%2").arg(resolution.width()).arg(resolution.height()),
                QVariant::fromValue(resolution)
            );
         _resolution_cb->setCurrentIndex(18); // 1280x720
    }
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
    if(resolution == currentResolution())
        // Skip a try to choose the same resolution.
        return;

    // Resolution is really supported by camera
    Q_ASSERT(qFind(_camera_image_capture->supportedResolutions(), resolution)
        != _camera_image_capture->supportedResolutions().end());

    QImageEncoderSettings image_settings;
    image_settings.setResolution(resolution);
    _camera->stop();
    _camera->unload();
    _camera_image_capture->setEncodingSettings(image_settings);
    _camera->load();
    _camera->start();

    emit resolutionChanged(resolution);
}

} // namespace laser_detector
