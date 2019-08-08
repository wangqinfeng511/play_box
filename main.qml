import QtQuick 2.9
import QtQuick.Controls 2.2

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("Scroll")
    Image {
        id:img
        anchors.fill: parent
        source: "";
    }
    Connections{
        target: Api
        onUpdate_image:{
            img.source="";
//            console.log("updage_image");
            img.source="image://image_src/aa";
        }
    }
}
