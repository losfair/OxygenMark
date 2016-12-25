var OxygenMarkLib = require("./OxygenMarkBundle.min.js");

function OxygenMark() {
    this.loadDocumentFromSource = OxygenMarkLib.cwrap("loadDocumentFromSource", "number", ["string"]);
    this.setDocumentParam = OxygenMarkLib.cwrap("setDocumentParam", null, ["number", "string", "string"]);
    this.renderToHtml = OxygenMarkLib.cwrap("renderToHtml", "string", ["number", "number"]);
    this.destroyDocument = OxygenMarkLib.cwrap("destroyDocument", null, ["number"]);
}
