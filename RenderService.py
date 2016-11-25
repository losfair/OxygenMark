import OxygenMarkRenderer
import flask
import json
import gevent.monkey
import gevent.pywsgi

server_addr = "127.0.0.1"
server_port = 7022

app = flask.Flask(__name__)

@app.route("/render", methods = ["POST"])
def on_render_request():
    req_data = json.loads(flask.request.get_data().decode("utf-8"))
    if req_data["type"] != "template":
        return flask.Response("Data type not supported")
    
    # We assume the service is running in a low privileged user 
    # and the request is made from localhost, so no path check is done.

    tpl_path = req_data["path"]
    if type(tpl_path) != str:
        return flask.Response("Invalid path");
    
    tpl_params = req_data["params"]
    if type(tpl_params) != dict:
        return flask.Response("Invalid params");
    
    try:
        render_whole_page = req_data["is_whole_page"]
        if render_whole_page == True:
            is_whole_page = True
        else:
            is_whole_page = False
    except KeyError:
        is_whole_page = True
    
    try:
        result = OxygenMarkRenderer.render_template(tpl_path, tpl_params, is_whole_page)
    except RuntimeError as e:
        return flask.Response("Exception caught while rendering: " + e.message)
    
    return flask.Response(result)

if __name__ == "__main__":
    gevent_server = gevent.pywsgi.WSGIServer((server_addr, server_port), app)
    gevent_server.serve_forever()
