function execLater(contextObject, delay, callback, args)
{
    var timer = Qt.createQmlObject("import QtQml 2.15; Timer { }", contextObject);
    timer.interval = delay === undefined ? 100 : delay
    timer.repeat = false
    timer.triggered.connect(() => {
        if (args)
            callback(args)
            else callback()
            timer.destroy()
    })
    timer.start()
}

function newAnnotation(parent, type, geometry, config) {
    if(!parent || !type || !geometry)
        return null

    let annot = Qt.createQmlObject("import io.scrite.components 1.0; Annotation { objectName: \"ica\" }", parent)
    annot.type = type
    annot.geometry = geometry
    if(config) {
        for(member in config)
            annot.setAttribute(member, config[member])
    }

    return annot
}

function bounded(min, val, max)
{
    return Math.min(Math.max(min, val), max)
}

function todayWithZeroTime()
{
    let today = new Date();
    today.setHours(0, 0, 0, 0);
    return today;
}

function formatDate(date)
{
    const months =
            [ "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" ];

    const day = date.getDate();
    const month = months[date.getMonth()];

    return day + " " + month;
}

function formatDateIncludingYear(date)
{
    return formatDate(date) + " " + date.getFullYear();
}

function formatDateRangeAsString(start_date, end_date)
{
    if (typeof end_date === "number") {
        const nrDays = end_date

        end_date = new Date(start_date)
        end_date.setDate(start_date.getDate() + nrDays)
    }

    if (start_date.getFullYear() === end_date.getFullYear()) {
        return formatDate(start_date) + " - " + formatDateIncludingYear(end_date)
    }

    return formatDateIncludingYear(start_date) + " - " + formatDateIncludingYear(end_date)
}

function daysSpanAsString(nrDays)
{
    let ret = ""
    if (nrDays < 0)
    {
        ret = "Already"
    }
    else if (nrDays === 0) { ret = "Today" }
    else if (nrDays === 1) { ret = "Tomorrow" }
    else
    {
        const years = Math.floor(nrDays / 365)
        const days = nrDays % 365
        if (years == 0)
        {
            ret = days + " days"
        }
        else
        {
            if (years === 1) {
                ret = "1 year"
            } else {
                ret = years + " years"
            }

            if (days > 1) {
                ret += ", and " + days + " days"
            } else if (days === 1) {
                ret += ", and 1 day"
            }
        }
    }

    return ret
}

function daysBetween(start_date, end_date)
{
    let from = new Date(start_date);
    let until = new Date(end_date);

    from.setHours(0, 0, 0, 0)
    until.setHours(0, 0, 0, 0)

    return Math.ceil((until - from) / (1000 * 60 * 60 * 24));
}

function dateSpanAsString(start_date, end_date)
{
    const nr_days_remaining = daysBetween(start_date, end_date)
    return daysSpanAsString(nr_days_remaining)
}

function toTitleCase(str)
{
    return str
            .toLowerCase() // Convert the entire string to lowercase
            .split(' ') // Split into words by spaces
            .map(word => word.charAt(0).toUpperCase()
                         + word.slice(1)) // Capitalize the first letter of each word
            .join(' '); // Join the words back with spaces
}

function validateEmail(email)
{
    const emailRegex =
            /^(([^<>()[\]\\.,;:\s@"]+(\.[^<>()[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;
    return emailRegex.test(email);
}
