
from flask import Flask, render_template
import json
app = Flask(__name__)

@app.route('/')
def hello_world():
    return 'Hello, World!\n'

welcome = "Welcome to 3ESE API!"

@app.route('/api/welcome/')
def api_welcome():
    return welcome
    
@app.route('/api/welcome/<int:index>', methods=['GET','POST'])
def api_welcome_index(index):
	if type(index) != int:
		abort(404)
	else:
		return json.dumps({"index": index, "val": welcome[index]}), {"Content-Type": "application/json"}


@app.route('/api/request/', methods=['GET', 'POST'])
@app.route('/api/request/<path>', methods=['GET','POST'])
#@app.route('/coucou', methods=['GET','POST'])
def api_request(path=None):

    resp = {
            "method":   request.method,
            "url" :  request.url,
            "path" : path,
            "args": request.args,
            "headers": dict(request.headers),
    }
    if request.method == 'POST':
        resp["POST"] = {
                "data" : request.get_json(),
                "args" : request.args.get('args'),
}
    return jsonify(resp)


@app.errorhandler(404)
def page_not_found(error):
    return render_template('page_not_found.html'), 404

