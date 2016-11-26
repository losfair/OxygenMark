function OxygenMark() {
    this.loadDocumentFromSource = Module.cwrap("loadDocumentFromSource", "number", ["string"]);
    this.setDocumentParam = Module.cwrap("setDocumentParam", null, ["number", "string", "string"]);
    this.renderToHtml = Module.cwrap("renderToHtml", "string", ["number", "number"]);
    this.destroyDocument = Module.cwrap("destroyDocument", null, ["number"]);
}