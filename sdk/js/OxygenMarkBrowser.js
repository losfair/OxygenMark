function OxygenMark() {
    this.prototype.loadDocumentFromSource = Module.cwrap("loadDocumentFromSource", "number", ["string"]);
    this.prototype.setDocumentParam = Module.cwrap("setDocumentParam", null, ["number", "string", "string"]);
    this.prototype.renderToHtml = Module.cwrap("renderToHtml", "string", ["number", "number"]);
    this.prototype.destroyDocument = Module.cwrap("destroyDocument", null, ["number"]);
}