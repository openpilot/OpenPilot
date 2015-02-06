import QtQuick 2.0
import "."

Item {
    id: sceneItem
    property variant sceneSize

    SvgElementImage {
        id: compass_fixed
        elementName: "compass-fixed"
        sceneSize: sceneItem.sceneSize

        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)
    }

    SvgElementImage {
        id: compass_plane
        elementName: "compass-plane"
        sceneSize: sceneItem.sceneSize

        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)
    }

    SvgElementImage {
        id: compass_wheel
        elementName: "compass-wheel"
        sceneSize: sceneItem.sceneSize

        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)

        rotation: -AttitudeState.Yaw
        transformOrigin: Item.Center

        smooth: true
    }

    SvgElementImage {
        id: compass_home
        elementName: "compass-home" // Cyan point
        sceneSize: sceneItem.sceneSize
        smooth: true

        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)

        property real home_degrees: 180/3.1415 * Math.atan2(TakeOffLocation.East - PositionState.East, TakeOffLocation.North - PositionState.North)

        rotation: -AttitudeState.Yaw + home_degrees
        transformOrigin: Item.Bottom
        visible: TakeOffLocation.Status == 0

    }

    SvgElementImage {
        id: compass_waypoint // Double Purple arrow
        elementName: "compass-waypoint"
        sceneSize: sceneItem.sceneSize

        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)

        property real course_degrees: 180/3.1415 * Math.atan2(PathDesired.End_East - PositionState.East, PathDesired.End_North - PositionState.North)

        rotation: -AttitudeState.Yaw + course_degrees
        transformOrigin: Item.Center

        smooth: true
        visible: PathDesired.End_East !== 0.0 && PathDesired.End_East !== 0.0
    }



    Item {
        id: compass_text_box

        property variant scaledBounds: svgRenderer.scaledElementBounds("pfd.svg", "compass-text")

        x: scaledBounds.x * sceneItem.width
        y: scaledBounds.y * sceneItem.height
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height

        Text {
            id: compass_text
            text: Math.floor(AttitudeState.Yaw).toFixed()
            color: "white"
            font {
                family: pt_bold.name
                pixelSize: parent.height * 1.2
            }
            anchors.centerIn: parent
        }
    }

}
