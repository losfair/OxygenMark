const OMRenderer = require("./OxygenMarkRenderer");

const ctx = new OMRenderer();

ctx.loadFile("./test.omt", {
    "text1": "Hello node",
	"id1": "id-1"
});
console.log(ctx.render());
let renderer = ctx.prepare();
console.log(renderer.toString());
console.log(renderer({
	"id2": "id-2"
}));
ctx.destroy();
