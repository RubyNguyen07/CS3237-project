import cv2
import mediapipe as md
import urllib.request
import numpy as np

import paho.mqtt.client as mqtt
from time import sleep

md_drawing=md.solutions.drawing_utils
md_drawing_styles=md.solutions.drawing_styles
md_pose=md.solutions.pose

url = 'http://url/cam-lo.jpg' ### Replace url with ESP32-CAM url ###
hand_raised = False


def on_connect(client, userdata, flags, rc):
    print("Connected with result code: " + str(rc))
    sleep(2)
    client.subscribe("parked/#")    

def on_message(client, userdata, message):
    global hand_raised
    if message.payload.decode('UTF-8') == "Parked":
        hand_raised = False
        pose_prediction()

def pose_prediction():
    global hand_raised
    # Accuracy of detection of features/coordinate
    with md_pose.Pose(min_detection_confidence=0.7, min_tracking_confidence=0.7) as pose:
        while True:
            # Download image from esp32cam url and decode image data into an image format that OpenCV can work with
            img_resp=urllib.request.urlopen(url)
            imgnp=np.array(bytearray(img_resp.read()),dtype=np.uint8)
            im = cv2.imdecode(imgnp,-1)
            # mediapipe only works on rgb images so need to convert from bgr(cv2)
            image=cv2.cvtColor(cv2.flip(im,1), cv2.COLOR_BGR2RGB)
            # apply pose model to image
            result=pose.process(image)
            # will be used to contain the 32 body points (elbow,knee etc)
            imlist=[]
                
            # check there is body in the video
            if result.pose_landmarks:
                # to draw the lines and dots
                md_drawing.draw_landmarks(
                        image, result.pose_landmarks, md_pose.POSE_CONNECTIONS)
                for id, im in enumerate(result.pose_landmarks.landmark):
                    # obtain height and width of video
                    h, w, _ = image.shape
                    # obtain exact coordinate for specific body point (by multiplying with the ratio)
                    X, Y = int(im.x*w), int(im.y*h)
                    # append body points and their exact coordinates (x,y)
                    imlist.append([id,X,Y])
            # Checks if anyone is raising right/left hand (determined when the wrist goes above the shoulder)
            if len(imlist) != 0:
                # uses the y coordinates of body point
                # Right wrist: 16, Right shoulder: 12, Left wrist: 15, Left shoulder: 11
                if imlist[16][2] < imlist[12][2] or imlist[15][2] < imlist[11][2]:
                    client.publish("start_A/now", "START")
                    hand_raised = True
    
            # Shows the video
            cv2.imshow("Customer", cv2.flip(image,1))

            # Exit condition
            key = cv2.waitKey(1) 
            if key == ord('q') or hand_raised == True:
                cv2.destroyAllWindows()
                break

            
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect("localhost", 1883, 60)
pose_prediction()
client.loop_forever()
	