import OxygenMarkRenderer

result = OxygenMarkRenderer.render_template("./test.omc", {
    "text1": "abc",
    "isInvisible": "false"
})

print(result)