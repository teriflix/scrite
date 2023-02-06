function execLater(contextObject, delay, callback, args) {
    var timer = Qt.createQmlObject("import QtQuick 2.0; Timer { }", contextObject);
    timer.interval = delay === undefined ? 100 : delay
    timer.repeat = false
    timer.triggered.connect(() => {
                                callback(args)
                                timer.destroy()
                            })
    timer.start()
}
