from flask import Flask, request, jsonify
import numpy as np
import joblib
from db import get_data, update_order, db_engine, get_all_orders
from datetime import datetime, timedelta
import psycopg2
from flask_cors import CORS


filename = "random_forest_model.joblib"
# Initialize Flask app
app = Flask(__name__)
CORS(app)  # Enables CORS for all routes and origins by default


loaded_model = joblib.load(filename)

conn = psycopg2.connect(db_engine())

def start_of_week(givenDate):
    # Adjust `given_date` to Monday (start of the week)
    date_object = datetime.strptime(givenDate, "%Y-%m-%d").date()
    print(date_object)
    startOfWeek = date_object - timedelta(days=date_object.weekday())
    return str(startOfWeek)

# API to order 
@app.route('/api/order', methods=['POST'])
def order():
    try: 
        data = request.get_json()
        if data is None: 
            return jsonify({"error": "No JSON data provided"}), 400
        data["startOfWeek"] = start_of_week(data["givenDate"])
        print(data)
        update_order(data, conn)
        return jsonify({'message': 'order updated'}), 200
    except Exception as e:
        print("Order error:", e)
        return jsonify({'error': 'Order error, make sure date format is Y-m-d'}), 500
    
# API to order 
@app.route('/api/order', methods=['GET'])
def get_orders():
    try: 
        all_orders = get_all_orders(conn)
        return jsonify({'orders': all_orders}), 200
    except Exception as e:
        print("Fetch order error:", e)
        return jsonify({'error': 'Cannot fetch orders'}), 500
    
    
# API to make predictions
@app.route('/api/predict', methods=['GET'])
def predict():
    try: 
        givenDate = request.args.get("date")
        if givenDate:
            startOfWeek = start_of_week(givenDate)
            input_data = get_data(startOfWeek, conn)
            if not input_data: 
                return jsonify({'error': 'There has not been any data yet'}), 400
            print(input_data["prev_week_1_co"])
            input_data = np.array([[int(input_data["prev_week_1_co"]), int(input_data["prev_week_1_fo"]), int(input_data["prev_week_1_so"]), int(input_data["prev_week_2_co"]), int(input_data["prev_week_2_fo"]), int(input_data["prev_week_2_so"])]])
            predicted = loaded_model.predict(input_data)[0]
            print(predicted)
            # Return result
            return jsonify({'predictedChickenOrder': predicted[0], 'predictedFishOrder': predicted[1], 'predictedSaladOrder': predicted[2]}), 200
        else:
            return jsonify({'error': 'Start of week needs to be provided'}), 400
    except Exception as e:
        print("Prediction error:", e)
        return jsonify({'error': 'Prediction error'}), 500
    
@app.route('/api/ping', methods=['GET'])
def ping():
    return "pong", 200

# Run the Flask app
if __name__ == '__main__':
    print("Hosted!")
    app.run(host='0.0.0.0', port=5001, debug=True)
