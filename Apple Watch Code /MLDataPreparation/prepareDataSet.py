#https://apple.github.io/turicreate/docs/userguide/activity_classifier/data-preparation.html

#https://stackoverflow.com/questions/7588934/how-to-delete-columns-in-a-csv-file

import pandas as pd

dataFrame = pd.read_csv("my_iOS_device 191221 12_08_32.csv")
dataFrame['sessionId'] = pd.Series(0, index = dataFrame.index)
dataFrame['activity'] = pd.Series("Spinning", index = dataFrame.index)
keep_col = ['loggingTime(txt)','activity','accelerometerAccelerationX(G)','accelerometerAccelerationY(G)','accelerometerAccelerationZ(G)','gyroRotationX(rad/s)','gyroRotationY(rad/s)','gyroRotationZ(rad/s)']
newDataFrame = dataFrame[keep_col]


newDataFrame.to_csv("newFile.csv", index=False)

#https://medium.com/skafosai/activity-classification-for-watchos-part-1-542d44388c40
