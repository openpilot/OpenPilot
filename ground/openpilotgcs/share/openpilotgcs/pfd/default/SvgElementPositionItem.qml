import QtQuick 2.0

Item {
    id: sceneItem
    property variant sceneSize
    property string elementName
    property string svgFileName: "pfd.svg"
    property variant scaledBounds: svgRenderer.scaledElementBounds(svgFileName, elementName)

    x: Math.floor(scaledBounds.x * sceneSize.width)
    y: Math.floor(scaledBounds.y * sceneSize.height)
    width: Math.floor(scaledBounds.width * sceneSize.width)
    height: Math.floor(scaledBounds.height * sceneSize.height)
}
