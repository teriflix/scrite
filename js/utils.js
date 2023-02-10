function execLater(contextObject, delay, callback, args) {
    var timer = Qt.createQmlObject("import QtQml 2.15; Timer { }", contextObject);
    timer.interval = delay === undefined ? 100 : delay
    timer.repeat = false
    timer.triggered.connect(() => {
                                if(args)
                                    callback(args)
                                else
                                    callback()
                                timer.destroy()
                            })
    timer.start()
}
