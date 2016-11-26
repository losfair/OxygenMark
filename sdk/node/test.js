var fs = require("fs");
var OxygenMark = require("./build/Release/OxygenMark");

var docContent = fs.readFileSync("./test.omt", "utf-8");

var startTime = Date.now();

var doc = OxygenMark.loadDocumentFromSource(docContent);
for(var i = 0; i < 10000; i++) {
    OxygenMark.setDocumentParam(doc, "text1", i);
    var result = OxygenMark.renderToHtml(doc, false);
}
OxygenMark.destroyDocument(doc);

var endTime = Date.now();

console.log(result);
console.log(endTime - startTime);
