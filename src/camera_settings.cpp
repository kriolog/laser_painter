#include "camera_settings.h"

#include <QComboBox>
#include <QCamera>
#include <QCameraInfo>
#include <QCameraImageCapture>
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
    _camera_image_capture(0)
{
    Q_ASSERT(video_frame_grabber);

    connect(this, &CameraSettings::cameraChanged, video_frame_grabber, &VideoFrameGrabber::installCamera);

    _camera_cb = new QComboBox();
//     _camera_cb->setEditable(true);
    connect(_camera_cb, SIGNAL(currentIndexChanged(int)), this, SLOT(changeCamera()));


    int cur_camera_id_to_set = -1;
//     _camera_cb->clear();
    foreach(const QCameraInfo& camera_info, QCameraInfo::availableCameras()) {
        _camera_cb->addItem(camera_info.description(), QVariant::fromValue(camera_info));
        if (camera_info == QCameraInfo::defaultCamera())
            cur_camera_id_to_set = _camera_cb->count() - 1;
    }

    if(cur_camera_id_to_set >= 0) {
        _camera_cb->setCurrentIndex(cur_camera_id_to_set);
        changeCamera();
    }

    QVBoxLayout* main_lo = new QVBoxLayout();
    setLayout(main_lo);
    main_lo->addWidget(_camera_cb);
}

void CameraSettings::updateAvailableCameras()
{
}

void CameraSettings::changeCamera()
{
    delete _camera_image_capture;
    delete _camera;

//     qDebug()<<_camera_cb->currentData().value<QCameraInfo>().deviceName();
    _camera = new QCamera(_camera_cb->currentData().value<QCameraInfo>());
    _camera_image_capture = new QCameraImageCapture(_camera);
    QImageEncoderSettings image_settings;
//     imageSettings.setCodec("image/jpeg");
    image_settings.setResolution(640, 480);
    _camera_image_capture->setEncodingSettings(image_settings);

    emit cameraChanged(_camera);
}

void CameraSettings::writeSettings() const
{
    // TODO
}

} // namespace laser_detector