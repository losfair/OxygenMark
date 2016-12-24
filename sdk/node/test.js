const OMRenderer = require("./OxygenMarkRenderer");

const cxt = new OMRenderer();

cxt.loadFile("./test.omt", {
    "text1": "Hello node"
});
console.log(cxt.render());
cxt.clearParams();
console.log(cxt.prepare().toString());
cxt.destroy();
