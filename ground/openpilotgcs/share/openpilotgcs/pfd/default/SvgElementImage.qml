import Qt 4.7

Image {
    id: sceneItem
    property variant sceneSize
    property string elementName
    property string svgFileName: "pfd.svg"
    property int vSlice: 0
    property int vSliceCount: 0
    property int hSlice: 0
    property int hSliceCount: 0
    property variant scaledBounds: svgRenderer.scaledElementBounds(svgFileName, elementName)

    sourceSize.width: Math.round(sceneSize.width*scaledBounds.width)
    sourceSize.height: Math.round(sceneSize.height*scaledBounds.height)

    Component.onCompleted: {
        var params = ""
        if (hSliceCount > 1)
            params += "hslice="+hSlice+":"+hSliceCount
        if (vSliceCount > 1)
            params += "vslice="+vSlice+":"+vSliceCount

        if (params != "")
            params = "?" + params

        source = "image://svg/"+svgFileName+"!"+elementName+params
    }
}
