var fs = require("fs");
var OxygenMark = require("./OxygenMarkBundle.min.js");

var loadDocumentFromSource = OxygenMark.cwrap("loadDocumentFromSource", "number", ["string"]);
var setDocumentParam = OxygenMark.cwrap("setDocumentParam", null, ["number", "string", "string"]);
var renderToHtml = OxygenMark.cwrap("renderToHtml", "string", ["number", "number"]);
var destroyDocument = OxygenMark.cwrap("destroyDocument", null, ["number"]);

var tpl = fs.readFileSync("./test.omt", "utf-8");

var startTime = Date.now();
var doc = loadDocumentFromSource(tpl);
setDocumentParam(doc, "text1", "T1");
var result = renderToHtml(doc, false);
destroyDocument(doc);
var endTime = Date.now();

console.log(result);
console.log(endTime - startTime);
