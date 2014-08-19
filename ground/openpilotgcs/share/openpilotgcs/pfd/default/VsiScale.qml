import QtQuick 2.0

Item {
    id: sceneItem
    property variant sceneSize

    SvgElementImage {
        id: vsi_window
        elementName: "vsi-window"
        sceneSize: sceneItem.sceneSize
        clip: true

        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)

    }

    SvgElementImage {
        id: vsi_waypoint
        elementName: "vsi-waypoint"
        sceneSize: sceneItem.sceneSize

        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height

        x: scaledBounds.x * sceneItem.width
        y: scaledBounds.y * sceneItem.height

        smooth: true
        visible: VelocityDesired.Down !== 0.0

        //rotate it around the center
        transform: Rotation {
            angle: -VelocityDesired.Down*5
            origin.y : vsi_waypoint.height/2 
            origin.x : vsi_waypoint.width*33
        }
    }

    SvgElementImage {
        id: vsi_scale

        elementName: "vsi-scale"
        sceneSize: sceneItem.sceneSize

        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)

    }

    SvgElementImage {
        id: vsi_arrow
        elementName: "vsi-arrow"
        sceneSize: sceneItem.sceneSize

        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height

        x: scaledBounds.x * sceneItem.width
        y: scaledBounds.y * sceneItem.height

        smooth: true

        //rotate it around the center
        transform: Rotation {
            angle: -VelocityState.Down*5
            origin.y : vsi_arrow.height/2 
            origin.x : vsi_arrow.width*3.15
        }
    }

    SvgElementImage {
        id: foreground
        elementName: "foreground"
        sceneSize: sceneItem.sceneSize

        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)

    }
}
