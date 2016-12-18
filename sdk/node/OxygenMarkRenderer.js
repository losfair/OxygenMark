const fs = require("fs");
const OxygenMark = require("./build/Release/OxygenMark");

function OxygenMarkRenderer() {
    this.docContext = null;
    this.setParams = (params) => {
        if(!this.docContext) {
            throw "Not initialized";
        }
        for(var k in params) {
            OxygenMark.setDocumentParam(this.docContext, k, params[k]);
        }
    }
    this.load = (src, params) => {
        if(this.docContext) {
            this.destroy();
        }
        this.docContext = OxygenMark.loadDocumentFromSource(src);
        if(params) {
            this.setParams(params);
        }
    }
    this.loadFile = (src, params) => {
        var docContent = fs.readFileSync(src, "utf-8");
        this.load(docContent, params);
    }
    this.loadCompiledFile = (src, params) => {
        this.docContext = OxygenMark.loadDocumentFromBinaryFile(src);
        if(params) {
            this.setParams(params);
        }
    }
    this.clearParams = () => {
        if(!this.docContext) {
            throw "Not initialized";
        }
        OxygenMark.clearDocumentParams(this.docContext);
    }
    this.destroy = () => {
        if(!this.docContext) return;
        OxygenMark.destroyDocument(this.docContext);
        this.docContext = null;
    }
    this.render = (isWholePage) => {
        if(isWholePage === undefined) isWholePage = true;
        if(!this.docContext) {
            throw "Not initialized";
        }
        return OxygenMark.renderToHtml(this.docContext, isWholePage);
    }
    this.prepareRaw = (isWholePage) => {
        if(isWholePage === undefined) isWholePage = true;
        if(!this.docContext) {
            throw "Not initialized";
        }
        return OxygenMark.generateRenderer(this.docContext, isWholePage);
    }
    this.prepare = (isWholePage) => {
        if(isWholePage === undefined) isWholePage = true;
        var generated = this.prepareRaw(isWholePage);
        if(!generated) return null;
        eval("var renderer = " + generated);
        return renderer;
    }
}

module.exports = OxygenMarkRenderer;