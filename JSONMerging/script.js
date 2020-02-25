const fs = require('fs');
var kinect;
var watch;
var merged = {
"session" : [{
  "session_id" : 0,
  "minHeartRate" : 0,
  "maxHeartRate" : 0,
  "averageHeartRate" : 0,
  "caloriesBurned" : 0,
  "distanceTravelled" : 0,
  "timestamp" : "N/A",
  "durationInSec" : 0,
  "avgJointDistMoved" : 0,
  "maxLeftHandHeight" : 0,
  "maxRightHandHeight" : 0,
  "maxLeftKneeHeight" : 0,
  "maxRightKneeHeight" : 0
}]
}

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
  if(kinect.session[0].session_id == watch.session[0].session_id){
    merged.session[0].session_id = kinect.session[0].session_id;
    merged.session[0].minHeartRate = watch.session[0].minHeartRate;
    merged.session[0].maxHeartRate = watch.session[0].maxHeartRate;
    merged.session[0].averageHeartRate = watch.session[0].averageHeartRate;
    merged.session[0].caloriesBurned = watch.session[0].caloriesBurned;
    merged.session[0].distanceTravelled = watch.session[0].distanceTravelled;
    merged.session[0].timestamp = kinect.session[0].timestamp;
    merged.session[0].durationInSec = kinect.session[0].durationInSec;
    merged.session[0].avgJointDistMoved = kinect.session[0].avgJointDistMoved;
    merged.session[0].maxLeftHandHeight = kinect.session[0].maxLeftHandHeight;
    merged.session[0].maxRightHandHeight = kinect.session[0].maxRightHandHeight;
    merged.session[0].maxLeftKneeHeight = kinect.session[0].maxLeftKneeHeight;
    merged.session[0].maxRightKneeHeight = kinect.session[0].maxRightKneeHeight;
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
var timestamp = merged.session[0].timestamp; //otherwise will use js Date.getTime();
var title = "Session_Summary_" + timestamp + ".json";

//console.log(title);

//console.log(JSON.stringify(merged));
fs.writeFile(title, JSON.stringify(merged, null, 2), function(err){
  if (err) {
    throw err;
    console.error("There was an issue saving the merged data");
  };
  console.log("Saved!");
});
