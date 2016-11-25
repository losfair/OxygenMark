var fs = require("fs");
var OxygenMark = require("./OxygenMarkBundle.min.js");

var loadDocumentFromSource = OxygenMark.cwrap("loadDocumentFromSource", "number", ["string"]);
var setDocumentParam = OxygenMark.cwrap("setDocumentParam", null, ["number", "string", "string"]);
var renderToHtml = OxygenMark.cwrap("renderToHtml", "string", ["number", "number"]);

var tpl = fs.readFileSync("./test.omt", "utf-8");
var doc = loadDocumentFromSource(tpl);
var result = renderToHtml(doc, false);

console.log(result);