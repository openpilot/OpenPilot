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

        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)

        smooth: true

        //rotate it around the center of horizon
        transform: Rotation {
            angle: -AttitudeState.Roll
            origin.y : rollscale.height*2.55
            origin.x : rollscale.width/2
        }
    }

}
