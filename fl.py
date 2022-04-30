from flask import Flask, render_template,request
from flask_socketio import SocketIO,emit

app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app,cors_allowed_origins="*")
@app.route("/")
def hello():
    return render_template("index.html")

@app.route("/good",methods=["POST","GET"])
def good():
    if request.method == "POST":
        name = request.form["name"]
        emit("icon_update",{"icon_src":name})
socketio.run(app,debug=True)
