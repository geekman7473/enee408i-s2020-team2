#!/usr/bin/python3
from .models import *
from .utils.utils import *
from .sort import *

import os, sys, time, datetime, random
import torch
from torch.utils.data import DataLoader
from torchvision import datasets, transforms
from torch.autograd import Variable

import matplotlib
matplotlib.use('GTK3Agg')
import matplotlib.pyplot as plt
import matplotlib.patches as patches

import cv2
import numpy as np

from PIL import Image

from .PathPlanning import plan_turn, run_plan

# These config and weight files need to be changed to match your current model
config_path='config/yolov3-tiny.cfg'
weights_path='config/yolov3_ckpt_19.pth'
class_path='config/coco.names'
img_size=416
# I found that adjusting the confidence threshold was very useful in eliminating false positives
conf_thres=0.80
nms_thres=0.4

# Load model and weights
device = torch.device("cuda")
#device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

model = Darknet(config_path, img_size=img_size).to(device)
model.load_state_dict(torch.load(weights_path))
Tensor = torch.cuda.FloatTensor
torch.set_default_tensor_type(torch.cuda.FloatTensor)
model.cuda()
model.eval()
classes = utils.load_classes(class_path)

def detect_image(img):
    # scale and pad image
    ratio = min(img_size/img.size[0], img_size/img.size[1])
    imw = round(img.size[0] * ratio)
    imh = round(img.size[1] * ratio)
    img_transforms = transforms.Compose([ transforms.Resize((imh, imw)),
         transforms.Pad((max(int((imh-imw)/2),0), max(int((imw-imh)/2),0), max(int((imh-imw)/2),0), max(int((imw-imh)/2),0)),
                        (128,128,128)),
         transforms.ToTensor(),
         ])
    # convert image to Tensor
    image_tensor = img_transforms(img).float()
    image_tensor = image_tensor.unsqueeze_(0).cuda()
    #if image_tensor.is_cuda:
      #print("IS CUDA")
    input_img = Variable(image_tensor.type(Tensor))
    # run inference on the model and get detections
    with torch.no_grad():
        detections = model(input_img)
        detections = utils.non_max_suppression(detections, conf_thres, nms_thres)
    return detections[0]
	
# This you can adjust to any video file of your choice
videopath = './videos/IMG_9663.mp4'

#%pylab inline 
#from IPython.display import clear_output

cmap = plt.get_cmap('tab20b')
colors = [cmap(i)[:3] for i in np.linspace(0, 1, 20)]

# initialize Sort object and video capture
vid = cv2.VideoCapture('/dev/video1')
mot_tracker = Sort() 

out = cv2.VideoWriter('output.avi',cv2.VideoWriter_fourcc('M','J','P','G'), 30, (1920,1080))

#while(True):

i = 0
import time
millis = int(round(time.time() * 1000))

fig=plt.figure(figsize=(12, 8))
fig, ax = plt.subplots()

plt.ion()
plt.show()

while (vid.isOpened()):
    print("FRAME")
    vid.grab()
    vid.grab()
    vid.grab()
    vid.grab()
    ret, frame = vid.read()
    if ret == False:
      break
    frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    pilimg = Image.fromarray(frame)
    detections = detect_image(pilimg)
    #
    img = np.array(pilimg)
    pad_x = max(img.shape[0] - img.shape[1], 0) * (img_size / max(img.shape))
    pad_y = max(img.shape[1] - img.shape[0], 0) * (img_size / max(img.shape))
    unpad_h = img_size - pad_y
    unpad_w = img_size - pad_x
    if detections is not None:
        print(detections)
    if False:
        tracked_objects = mot_tracker.update(detections.cpu())
        #
        unique_labels = detections[:, -1].cpu().unique()
        n_cls_preds = len(unique_labels)
        for x1, y1, x2, y2, obj_id, cls_pred in tracked_objects:
            box_h = int(((y2 - y1) / unpad_h) * img.shape[0])
            box_w = int(((x2 - x1) / unpad_w) * img.shape[1])
            y1 = int(((y1 - pad_y // 2) / unpad_h) * img.shape[0])
            x1 = int(((x1 - pad_x // 2) / unpad_w) * img.shape[1])
            #
            color = colors[int(obj_id) % len(colors)]
            color = [i * 255 for i in color]
            cls = classes[int(cls_pred)]
            cv2.rectangle(frame, (x1, y1), (x1+box_w, y1+box_h), color, 4)
            cv2.rectangle(frame, (x1, y1-35), (x1+len(cls)*19+60, y1), color, -1)
            cv2.putText(frame, cls + "-" + str(int(obj_id)), (x1, y1 - 10), cv2.FONT_HERSHEY_SIMPLEX, 1, (255,255,255), 3)
    i = i + 1
    print("FPS: " + str(i / ((int(round(time.time() * 1000)) - millis) / 1000)))
    #cv2.imshow('Frame', frame)
    #cv2.waitKey(1)
    # I elected to not output the results here in the notebook for performance,
    # but you can feel free to uncomment these lines if you want a live view of your model
    #out.write(frame)
    ax = fig.gca()
    #ax.title("Video Stream")
    ax.imshow(frame)
    plt.draw()
    plt.pause(.0001)
    #img=cv2.imread('image.jpg',1)
    #you can use:
    #img2 = Image.fromarray(frame, 'RGB')
    #img2.show()
    #clear_output(wait=True)
#out.release()
vid.release()
