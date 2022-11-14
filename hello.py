from flask import Flask, render_template,jsonify,request
import json
import serial

ser = serial.Serial("/dev/ttyAMA0",9600)

app = Flask(__name__)

temp = []

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
               	}
	return jsonify(resp)


@app.route('/api/temp', methods=['GET','POST'])
@app.route('/api/temp/<path>', methods=['GET','POST'])
def api_temp(path=None):
	respo = {
		"température = ":  temp
	}
	if request.method == 'POST':
		respo["POST"] = {
			"demands" : request.get_json(),
		}
		print("GET_T")
		new_temp = ser.read()
		print("reading OK")
		temp.append(new_temp)
	return jsonify(respo)

@app.errorhandler(404)
def page_not_found(error):
    return render_template('page_not_found.html'), 404

