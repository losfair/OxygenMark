const OMRenderer = require("./OxygenMarkRenderer");

const cxt = new OMRenderer();

var startTime = Date.now();
for(var i = 0; i < 10000; i++) {
    cxt.loadCompiledFile("../../test.omc", {
        "text1": i
    });
}
var endTime = Date.now();

console.log(cxt.render());
console.log(endTime - startTime);

cxt.destroy();
