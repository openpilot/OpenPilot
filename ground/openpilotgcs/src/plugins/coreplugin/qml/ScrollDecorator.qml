import QtQuick 1.1

Rectangle {
    id: scrollDecorator

    property Flickable flickableItem: null

    Loader {
        id: scrollLoader
        sourceComponent: scrollDecorator.flickableItem ? scrollBar : undefined
    }

    Component.onDestruction: scrollLoader.sourceComponent = undefined

    Component {
        id: scrollBar
        Rectangle {
            property Flickable flickable: scrollDecorator.flickableItem

            parent: flickable
            anchors.right: parent.right

            smooth: true
            radius: 2
            color: "gray"
            border.color: "lightgray"
            border.width: 1.0
            opacity: flickable.moving ? 0.8 : 0.4

            Behavior on opacity {
                NumberAnimation { duration: 500 }
            }

            width: 4
            height: flickable.height * (flickable.height / flickable.contentHeight)
            y: flickable.height * (flickable.contentY / flickable.contentHeight)
            visible: flickable.height < flickable.contentHeight
        }
    }
}
