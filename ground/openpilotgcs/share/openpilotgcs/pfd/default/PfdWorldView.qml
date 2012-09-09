import Qt 4.7

Item {
    id: worldView

    SvgElementImage {
        id: world
        elementName: "world"
        //worldView is loaded with Loader, so background element is visible
        sceneSize: background.sceneSize

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
