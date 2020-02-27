const fs = require('fs');
var kinect;
var watch;
var merged = {"id" : "0",
"watchDuration" : "0",
"minHeartRate" : "0",
"maxHeartRate" : "0",
"averageHeartRate" : "0",
"caloriesBurned" : "0",
"distanceTravelled" : "0",
"twists" : "0",
"timestamp" : "NA",
"kinectDurationInSec" : "0",
"avgJointDistMoved" : "0",
"maxLeftHandHeight" : "0",
"maxRightHandHeight" : "0",
"maxLeftKneeHeight" : "0",
"maxRightKneeHeight" : "0"}

//I tried refactoring this but failed miserably lol
//Putting the repeating code in a function causes scope errors, will refactor and fix once functional verion of this is produced

try {
  kinect = JSON.parse(fs.readFileSync("kinect.json"));
} catch (err){
  try {
  if (fs.existsSync(path)) {
    console.error("Kinect File could not be parsed");
    }
  } catch(err) {
    console.error("Kinect File could not be found");
  }
}

try {
  watch = JSON.parse(fs.readFileSync("watch.json"));
} catch (err){
  try {
  if (fs.existsSync(path)) {
    console.error("Watch File could not be parsed");
    }
  } catch(err) {
    console.error("Watch File could not be found");
  }
}

//console.log(kinect);
//console.log(watch);

if (kinect != undefined && watch != undefined){
  if(kinect.id == watch.id){
    merged.id = kinect.id;
    merged.watchDuration = watch.watchDuration;
    merged.minHeartRate = watch.minHeartRate;
    merged.maxHeartRate = watch.maxHeartRate;
    merged.averageHeartRate = watch.averageHeartRate;
    merged.caloriesBurned = watch.caloriesBurned;
    merged.distanceTravelled = watch.distanceTravelled;
    merged.twists = watch.twists;
    merged.timestamp = kinect.timestamp;
    merged.durationInSec = kinect.durationInSec;
    merged.avgJointDistMoved = kinect.avgJointDistMoved;
    merged.maxLeftHandHeight = kinect.maxLeftHandHeight;
    merged.maxRightHandHeight = kinect.maxRightHandHeight;
    merged.maxLeftKneeHeight = kinect.maxLeftKneeHeight;
    merged.maxRightKneeHeight = kinect.maxRightKneeHeight;
  }
  else {
    console.error("The session IDs do not match: the session will not be saved.");
  }
}
else if (kinect == undefined){
  console.error("Only the watch data will be saved in this session");
  merged = watch;
  //timestamp code goes here if AppleWatch can't save a time
}
else if (watch == undefined){
  console.error("Only the kinect data will be saved in this session");
  merged = kinect;
}
else {
  console.error("No data was received");
}

//here, we delete the original files for kinect and watch:

fs.unlink("kinect.json", function (err) {
  if (err){
    throw err;
    console.error("could not delete kinect.json");
  }
  console.log('File deleted!');
})
fs.unlink("watch.json", function (err) {
  if (err){
    throw err;
    console.error("could not delete watch.json");
  }
  console.log('File deleted!');
})

//console.log(merged);

//if can get same format timestamp from the AppleWatch as the Kinect!
var title = "Session_Summary_" + merged.timestamp + ".json";

//console.log(title);

//console.log(JSON.stringify(merged));
fs.writeFile(title, JSON.stringify(merged, null, 2), function(err){
  if (err) {
    throw err;
    console.error("There was an issue saving the merged data");
  };
  console.log("Saved!");
});
