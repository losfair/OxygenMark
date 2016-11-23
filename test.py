import OxygenMarkRenderer

result = OxygenMarkRenderer.render_template("./test.smc", {
    "text1": "abc",
    "isInvisible": "true"
})

print result