import Qt 4.7
import "."

Item {
    id: sceneItem
    property variant sceneSize

    SvgElementImage {
        id: compass
        elementName: "compass"
        sceneSize: sceneItem.sceneSize

        clip: true

        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)
        //anchors.horizontalCenter: parent.horizontalCenter

        //split compass band to 8 parts to ensure it doesn't exceed the max texture size
        Row {
            anchors.centerIn: parent
            //the band is 540 degrees wide, AttitudeActual.Yaw is converted to -180..180 range
            anchors.horizontalCenterOffset: -1*((AttitudeActual.Yaw+180+720) % 360 - 180)/540*width

            Repeater {
                model: 5
                SvgElementImage {
                    id: compass_band
                    elementName: "compass-band"
                    sceneSize: background.sceneSize
                    hSliceCount: 5
                    hSlice: index
                }
            }
        }
    }
}
