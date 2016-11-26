var OxygenMarkLib = require("./OxygenMarkBundle.min.js");

function OxygenMark() {
    this.prototype.loadDocumentFromSource = OxygenMarkLib.cwrap("loadDocumentFromSource", "number", ["string"]);
    this.prototype.setDocumentParam = OxygenMarkLib.cwrap("setDocumentParam", null, ["number", "string", "string"]);
    this.prototype.renderToHtml = OxygenMarkLib.cwrap("renderToHtml", "string", ["number", "number"]);
    this.prototype.destroyDocument = OxygenMarkLib.cwrap("destroyDocument", null, ["number"]);
}