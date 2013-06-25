import Qt 4.7
import org.OpenPilot 1.0

OsgEarth {
    id: earthView

    sceneFile: qmlWidget.earthFile
    fieldOfView: 90

    yaw: AttitudeState.Yaw
    pitch: AttitudeState.Pitch
    roll: AttitudeState.Roll

    latitude: qmlWidget.actualPositionUsed ?
                  GPSPositionSensor.Latitude/10000000.0 : qmlWidget.latitude
    longitude: qmlWidget.actualPositionUsed ?
                   GPSPositionSensor.Longitude/10000000.0 : qmlWidget.longitude
    altitude: qmlWidget.actualPositionUsed ?
                  GPSPositionSensor.Altitude : qmlWidget.altitude
}
