import Qt 4.7

Image {
    id: sceneItem
    property variant sceneSize
    property string elementName
    property string svgFileName : "pfd.svg"
    property variant scaledBounds: svgRenderer.scaledElementBounds(svgFileName, elementName)

    sourceSize.width: Math.round(sceneSize.width*scaledBounds.width)
    sourceSize.height: Math.round(sceneSize.height*scaledBounds.height)

    Component.onCompleted: source = "image://svg/"+svgFileName+"!"+elementName
}
