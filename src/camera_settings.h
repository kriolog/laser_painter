#ifndef CAMERA_SETTINGS
#define CAMERA_SETTINGS

#include <QWidget>
#include <QCameraInfo>
#include <QSize>

namespace laser_painter {
    class VideoFrameGrabber;
}

class QComboBox;
class QCamera;
class QCameraImageCapture;

namespace laser_painter {

class CameraSettings: public QWidget
{
    Q_OBJECT

public:
    explicit CameraSettings(
        VideoFrameGrabber* video_frame_grabber,
        QWidget* parent = 0, Qt::WindowFlags f = 0
    );

    // TODO: last camera and its resolution.
    // void writeSettings() const;

signals:
    void cameraChanged(QCamera* camera);
    void resolutionChanged(const QSize& resolution);

private slots:
    // Update available cameras and set a default camera (if any) or a first
    // available camera (if any) if @param try_set_camera is true.
    void updateAvailableCameras(bool try_set_camera = false);
    // Update a current camera resolutions and choose the highest resolution.
    void updateSupportedResolutions();
    // Change camera and install the highest resolution.
    void changeCamera();
    void changeResolution();

private:
    QComboBox* _camera_cb;
    QComboBox* _resolution_cb;
    QCamera* _camera;
    QCameraImageCapture* _camera_image_capture;
    QCameraInfo _camera_info;
    QSize _camera_resolution;

};

} // namespace laser_painter

#endif // CAMERA_SETTINGS