import cv2
import numpy as np
from matplotlib import pyplot as plt

img1 = cv2.imread("/home/pi/Pictures/pic1.jpg")
img2 = cv2.imread("/home/pi/Pictures/pic2.jpg")
img3 = cv2.imread("/home/pi/Pictures/pic3.jpg")
img4 = cv2.imread("/home/pi/Pictures/potato.jpg")

img1 = cv2.resize(img1,(160,120))
img2 = cv2.resize(img2, (160,120))
img3 = cv2.resize(img3,(160,120))
img4 = cv2.resize(img4, (160,120))

img1=cv2.copyMakeBorder(img1, 30, 0, 15, 15, borderType=cv2.BORDER_CONSTANT)
img2=cv2.copyMakeBorder(img2, 10, 0, 15, 15, borderType=cv2.BORDER_CONSTANT)
img3=cv2.copyMakeBorder(img3, 10, 0, 15, 15, borderType=cv2.BORDER_CONSTANT)
img4=cv2.copyMakeBorder(img4, 10, 0, 15, 15, borderType=cv2.BORDER_CONSTANT)

addv = cv2.vconcat([img1, img2])
addv2 =cv2.vconcat([img3, img4])
result = cv2.vconcat([addv, addv2])
final = cv2.hconcat([result, result])

cv2.imwrite('final.jpg',final)

