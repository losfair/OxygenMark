const OMRenderer = require("./OxygenMarkRenderer");

const cxt = new OMRenderer();

cxt.loadCompiledFile("../../test.omc", {
    "text1": "Hello node"
});
console.log(cxt.render());
cxt.clearParams();
console.log(cxt.prepare());
cxt.destroy();
