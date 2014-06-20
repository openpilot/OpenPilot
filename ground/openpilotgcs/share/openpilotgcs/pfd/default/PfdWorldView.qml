import QtQuick 2.0

Item {
    id: worldView
    property real horizontCenter : horizontCenterItem.horizontCenter

    Rectangle {
        // using rectange instead of svg rendered to pixmap
        // as it's much more memory efficient
        id: world
        smooth: true

        property variant scaledBounds: svgRenderer.scaledElementBounds("pfd.svg", "horizon")
        width: Math.round(sceneItem.width*scaledBounds.width/2)*2
        height: Math.round(sceneItem.height*scaledBounds.height/2)*3

        property double pitch1DegScaledHeight: (svgRenderer.scaledElementBounds("pfd.svg", "pitch-90").y -
                                                svgRenderer.scaledElementBounds("pfd.svg", "pitch90").y)/180.0

        property double pitch1DegHeight: sceneItem.height*pitch1DegScaledHeight

        gradient: Gradient {
            GradientStop { position: 0.4999;   color: "#0164CC" }
            GradientStop { position: 0.5001;   color: "#653300" }
        }

        transform: [
            Translate {
                id: pitchTranslate
                x: Math.round((world.parent.width - world.width)/2)
                // y is centered around world_center element
                y: Math.round(horizontCenter - world.height/2 +
                              AttitudeState.Pitch*world.pitch1DegHeight)
            },
            Rotation {
                angle: -AttitudeState.Roll
                origin.x : world.parent.width/2
                origin.y : horizontCenter
            }
        ]

        SvgElementImage {
            id: horizont_line
            elementName: "center-line"

            //worldView is loaded with Loader, so background element is visible
            sceneSize: background.sceneSize
            anchors.centerIn: parent
            border: 1
            smooth: true
        }

        SvgElementImage {
            id: pitch_0
            elementName: "pitch0"

            sceneSize: background.sceneSize
            anchors.centerIn: parent
            border: 1            
            smooth: true
        }
    }

    Item {
        id: pitch_window
        property variant scaledBounds: svgRenderer.scaledElementBounds("pfd.svg", "pitch-window")

        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)
        width: Math.floor(scaledBounds.width * sceneItem.width)
        height: Math.floor(scaledBounds.height * sceneItem.height)

        rotation: -AttitudeState.Roll
        transformOrigin: Item.Center

        smooth: true
        clip: true

        SvgElementImage {
            id: pitch_scale
            elementName: "pitch-scale"
            //worldView is loaded with Loader, so background element is visible
            sceneSize: background.sceneSize
            anchors.centerIn: parent
            //see comment for world transform
            anchors.verticalCenterOffset: AttitudeState.Pitch*world.pitch1DegHeight
            border: 64 //sometimes numbers are excluded from bounding rect

            smooth: true
        }
    }

}
