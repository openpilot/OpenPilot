import Qt 4.7

Item {
    //worldView should fill the source size of svg document
    id: worldView

    Image {
        id: world
        source: "image://svg/pfd.svg!world"

        sourceSize.width: worldView.width
        sourceSize.height: worldView.height

        smooth: true

        transform: [
            Translate {
                id: pitchTranslate
                x: (world.parent.width - world.width)/2
                y: (world.parent.height - world.height)/2 + AttitudeActual.Pitch*world.parent.height/94
            },
            Rotation {
                angle: -AttitudeActual.Roll
                origin.x : world.parent.width/2
                origin.y : world.parent.height/2
            }
        ]
    }
}
