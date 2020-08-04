function fetchOpenGraphAttributes() {
    var attribs = {}

    var fetchMetaProperty = function(prop) {
        var element = document.querySelector('meta[property~="og:' + prop + '"]');
        return element ? element.getAttribute("content") : ""
    }

    var title = fetchMetaProperty("title")
    if(title === "")
        title = document.title

    attribs["url"] = location.href;
    attribs["type"] = fetchMetaProperty("type");
    attribs["title"] = title;
    attribs["image"] = fetchMetaProperty("image");
    attribs["description"] = fetchMetaProperty("description");

    return attribs;
}

fetchOpenGraphAttributes()
