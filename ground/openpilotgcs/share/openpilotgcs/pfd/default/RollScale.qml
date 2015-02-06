import QtQuick 2.0
import "."

Item {
    id: sceneItem
    property variant sceneSize
    property real horizontCenter

    onHorizontCenterChanged: console.log("horizont center:"+horizontCenter)

    SvgElementImage {
        id: rollscale
        elementName: "roll-scale"
        sceneSize: sceneItem.sceneSize

        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height

        x: scaledBounds.x * sceneItem.width
        y: scaledBounds.y * sceneItem.height

        smooth: true

        //rotate it around the center of horizon
        transform: Rotation {
            angle: -AttitudeState.Roll
            origin.y : rollscale.height*2.4
            origin.x : rollscale.width/2
        }
    }

}
