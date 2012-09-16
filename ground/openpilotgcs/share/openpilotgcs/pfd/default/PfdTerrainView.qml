import Qt 4.7
import org.OpenPilot 1.0

OsgEarth {
    id: earthView

    sceneFile: qmlWidget.earthFile
    fieldOfView: 90

    yaw: AttitudeActual.Yaw
    pitch: AttitudeActual.Pitch
    roll: AttitudeActual.Roll

    latitude: qmlWidget.actualPositionUsed ?
                  GPSPosition.Latitude/10000000.0 : qmlWidget.latitude
    longitude: qmlWidget.actualPositionUsed ?
                   GPSPosition.Longitude/10000000.0 : qmlWidget.longitude
    altitude: qmlWidget.actualPositionUsed ?
                  GPSPosition.Altitude : qmlWidget.altitude
}
