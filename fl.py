from flask import Flask, render_template,request
from flask_socketio import SocketIO

app = Flask(__name__,static_folder="./templates/images")
app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app,cors_allowed_origins="*")
@app.route("/")
def hello():
    return render_template("index.html")

@app.route("/get")
def get():
    if request.method == "GET":
        name = request.args["name"]
        print(name)
        socketio.emit("icon_update",{"icon_src":name})
        return "ok"
socketio.run(app,debug=True)
