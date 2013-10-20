import QtQuick 1.1

Item {
    id: sceneItem
    property variant sceneSize
    property real horizontCenter : world_center.y + world_center.height/2

    SvgElementImage {
        id: world_center
        elementName: "center-arrows"
        sceneSize: background.sceneSize

        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)
    }

    SvgElementImage {
        id: world_center_plane
        elementName: "center-plane"
        sceneSize: background.sceneSize

        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)
    }
}
