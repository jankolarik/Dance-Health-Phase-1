//
//  DanceSession.swift
//  DanceHeatlhTracking WatchKit Extension
//
//  Created by Honza on 06/01/2020.
//  Copyright © 2020 Team32. All rights reserved.
//
//  Adapted from SpeedySloth example from Apple


import WatchKit
import Foundation
import CoreMotion
import HealthKit
import CoreML

class DanceSession: WKInterfaceController, HKWorkoutSessionDelegate, HKLiveWorkoutBuilderDelegate {
    
// MARK: - Declaration of variables + actions
    //Need to replace '!' with '?' and handle errors!
    //Maybe move code to seperate classes?

    @IBOutlet weak var timer: WKInterfaceTimer!

    @IBOutlet weak var heartRateLabel: WKInterfaceLabel!
    @IBOutlet weak var caloriesLabel: WKInterfaceLabel!
    @IBOutlet weak var distanceLabel: WKInterfaceLabel!
    @IBOutlet weak var spinsLabel: WKInterfaceLabel!
    @IBOutlet weak var startDancingButton: WKInterfaceButton!
    

    var id = ""
    
    var healthStore = HKHealthStore()
    var motion = CMMotionManager()
    var session: HKWorkoutSession!
    var builder: HKLiveWorkoutBuilder!
    var configuration: HKWorkoutConfiguration!
    
    var currentState : Int = 0
    var noOfSpinsCompleted : Int = 0
    var isAutorotating : Bool = true
    
    var avgHeartRate : Double = 0
    var minHeartRate : Double = 0
    var maxHeartRate : Double = 0
    var distanceTravelled : Double = 0
    var caloriesBurned : Double = 0
    
    @IBAction func startDancingButtonClick() {
        if(currentState == 0){
            startSession()
            currentState = 1
            startDancingButton.setTitle("Stop Measuring")
        }
        else{
            endSession()
            currentState = 0
            startDancingButton.setTitle("Start Measuring")
        }
    }
    
    // MARK: - Default Interface Setup
    override func awake(withContext context: Any?) {
        super.awake(withContext: context)
        id = context as! String
        
        // Configure interface objects here.
        authenticator()
        setUpEnv()
    }
    
    
    override func willActivate() {
        // This method is called when watch view controller is about to be visible to user
        super.willActivate()
    }
    
    override func didDeactivate() {
        // This method is called when watch view controller is no longer visible
        super.didDeactivate()
    }
    
// MARK: - Session Management
    func startSession(){
        createHKWorkoutSession()
        currentState = 1
        startDancingButton.setTitle("Stop Dancing")
        motionData()
    }
        
    func endSession(){
        session.end()
        self.setDurationTimerDate(.ended)
        endCollectionOfWorkout()
        currentState = 0
        motionData()
        startDancingButton.setTitle("Start Dancing")
        postData()
        self.noOfSpinsCompleted = 0
    }
    
    //Used from SpeedySloth example code
     func setDurationTimerDate(_ sessionState: HKWorkoutSessionState) {
         /// Obtain the elapsed time from the workout builder.
         /// - Tag: ObtainElapsedTime
        let timerDate = Date(timeInterval: -self.builder.elapsedTime, since: Date())
         
         // Dispatch to main, because we are updating the interface.
         DispatchQueue.main.async {
            self.timer.setDate(timerDate)
         }
         
         // Dispatch to main, because we are updating the interface.
         DispatchQueue.main.async {
             /// Update the timer based on the state we are in.
             /// - Tag: UpdateTimer
             sessionState == .running ? self.timer.start() : self.timer.stop()
         }
     }
    
// MARK: - HealthKit Management
    
    //https://developer.apple.com/documentation/healthkit/setting_up_healthkit
    func authenticator(){
        // The quantity type to write to the health store.
        let typesToShare: Set = [
            HKQuantityType.workoutType()
        ]

        // The quantity types to read from the health store.
        let typesToRead: Set = [
            //HKQuantityType.workoutType()
            HKQuantityType.quantityType(forIdentifier: .heartRate)!,
            HKQuantityType.quantityType(forIdentifier: .distanceWalkingRunning)!,
            HKQuantityType.quantityType(forIdentifier: .activeEnergyBurned)!,
            HKQuantityType.quantityType(forIdentifier: .stepCount)!
        ]

        // Request authorization for those quantity types.
        healthStore.requestAuthorization(toShare: typesToShare, read: typesToRead) { (success, error) in
            // Handle error. No error handling in this sample project.
        }
    }
    
    func setUpEnv(){
        configuration = HKWorkoutConfiguration()
        configuration.activityType = .dance
        configuration.locationType = .indoor
    }
    
    func createHKWorkoutSession(){
        do {
            session = try HKWorkoutSession(healthStore: healthStore, configuration: configuration)
            builder = session.associatedWorkoutBuilder()
        } catch {
            // Handle failure here.
            return
        }
        
        // Setup session and builder.
        session?.delegate = self
        builder?.delegate = self
        
        builder.dataSource = HKLiveWorkoutDataSource(healthStore: healthStore,
        workoutConfiguration: configuration)
        
        
        session.startActivity(with: Date())
        builder.beginCollection(withStart: Date()) { (success, error) in
            self.setDurationTimerDate(.running)
        }
    }
    
    func endCollectionOfWorkout(){
        builder.endCollection(withEnd: Date()) { (success, error) in
            self.builder.finishWorkout { (workout, error) in
                // Dispatch to main, because we are updating the interface.
                DispatchQueue.main.async() {
                    self.dismiss()
                }
            }
        }
    }
    
    
    
    // MARK: - Motion Manager


    //https://stackoverflow.com/questions/46756274/how-to-work-with-accelerometer-data-from-the-apple-watch
    
    func motionData(){
        if(self.currentState == 1){
            motion.deviceMotionUpdateInterval = 0.3

            motion.startDeviceMotionUpdates(to: OperationQueue.current!) { (data, error) in
                if let myData = data {
                    if(abs(myData.rotationRate.x) > 7 && abs(myData.gravity.y) < 0.2){
                        self.noOfSpinsCompleted += 1
                    }
                    self.spinsLabel.setText("Spins: \(self.noOfSpinsCompleted)")
                }
            }
        }
        else{
            self.spinsLabel.setText("Spins: \(self.noOfSpinsCompleted)")
            motion.stopDeviceMotionUpdates()
            return
        }
    }
    

    // MARK: - HKLiveWorkoutBuilderDelegate
    
    func workoutBuilder(_ workoutBuilder: HKLiveWorkoutBuilder, didCollectDataOf collectedTypes: Set<HKSampleType>) {
            
        //Because we are updating to the interface
        DispatchQueue.main.async {
            if let heartRateType = HKQuantityType.quantityType(forIdentifier: .heartRate), let statistics = workoutBuilder.statistics(for: heartRateType){
                let heartRateUnit = HKUnit.count().unitDivided(by: HKUnit.minute())
                let value = statistics.mostRecentQuantity()?.doubleValue(for: heartRateUnit)
                let roundedValue = Double( round( 1 * value! ) / 1 )
                self.heartRateLabel.setText("\(roundedValue) BPM")
                self.avgHeartRate = roundedValue
                
                let minValue = statistics.minimumQuantity()?.doubleValue(for: heartRateUnit)
                let minRoundedValue = Double( round( 1 * minValue! ) / 1 )
                self.minHeartRate = minRoundedValue
                
                let maxValue = statistics.maximumQuantity()?.doubleValue(for: heartRateUnit)
                let maxRoundedValue = Double( round( 1 * maxValue! ) / 1 )
                self.maxHeartRate = maxRoundedValue
            }
             
            if let distanceType = HKQuantityType.quantityType(forIdentifier: .distanceWalkingRunning), let statistics = workoutBuilder.statistics(for: distanceType){
                let meterUnit = HKUnit.meter()
                let value = statistics.sumQuantity()?.doubleValue(for: meterUnit)
                let roundedValue = Double( round( 1 * value! ) / 1 )
                self.distanceLabel.setText("\(roundedValue) m")
                self.distanceTravelled = roundedValue
            }
            
            if let calorieType = HKQuantityType.quantityType(forIdentifier: .activeEnergyBurned), let statistics = workoutBuilder.statistics(for: calorieType){
                let energyUnit = HKUnit.kilocalorie()
                let value = statistics.sumQuantity()?.doubleValue(for: energyUnit)
                let roundedValue = Double( round( 1 * value! ) / 1 )
                self.caloriesLabel.setText("\(roundedValue) cal")
                self.caloriesBurned = roundedValue
            }
        }
    }
    
    // MARK: - Update the interface
    
    // Track elapsed time.
    func workoutBuilderDidCollectEvent(_ workoutBuilder: HKLiveWorkoutBuilder) {

        // Retreive the workout event.
        guard let workoutEventType = workoutBuilder.workoutEvents.last?.type else { return }
        
        // Update the timer based on the event received.
        switch workoutEventType {
        case .pause: // The user paused the workout.
            setDurationTimerDate(.paused)
        case .resume: // The user resumed the workout.
            setDurationTimerDate(.running)
        default:
            return
        }
    }
    
    // MARK: - HKWorkoutSessionDelegate
    func workoutSession(
        _ workoutSession: HKWorkoutSession,
        didChangeTo toState: HKWorkoutSessionState,
        from fromState: HKWorkoutSessionState,
        date: Date
    ) {
        // Dispatch to main, because we are updating the interface.
        DispatchQueue.main.async {}
    }
    
    func workoutSession(_ workoutSession: HKWorkoutSession, didFailWithError error: Error) {
        // No error handling in this sample project.
    }

// MARK: - Creating URL Requests
    //Mongo DB Docs
    //https://docs.mongodb.com/stitch/services/http-actions/http.post/
    func postData(){
        let today = Date()
        let formatter = DateFormatter()
        formatter.dateFormat = "dd-MM-yyyy"

        
        let jsonDanceObject: [String: Any] = [
            "id": id,
            "duration": String(format: "%.01f", self.builder.elapsedTime),
            "minHeartRate": minHeartRate,
            "maxHeartRate": maxHeartRate,
            "averageHeartRate": avgHeartRate,
            "caloriesBurned": caloriesBurned,
            "distanceTravelled": distanceTravelled,
            "twists" : noOfSpinsCompleted,
            "timeStamp" : formatter.string(from: today),
            "durationInSec" : -1,
            "avgJointDistanceMoved" : -1,
            "maxLeftHandHeight" : -1,
            "maxRightHandHeight" : -1,
            "maxLeftKneeHeight" : -1,
            "maxRightKneeHeight": -1,
            "linkToVideo": -1
        ]
        
        
        let jsonDanceObjectSerialized = try? JSONSerialization.data(withJSONObject: jsonDanceObject)
        
        //https://learnappmaking.com/urlsession-swift-networking-how-to/
        //Create new betton to send data (Provide success/failure of data sent?
        
        let session = URLSession.shared
        let url = URL(string: "http://51.11.52.98:3300/dance")!
        var request = URLRequest(url: url)
        request.httpMethod = "POST"
        request.httpBody = jsonDanceObjectSerialized
        request.setValue("application/json", forHTTPHeaderField: "Content-Type")

        
        let task = session.uploadTask(with: request, from: jsonDanceObjectSerialized) { data, response, error in
            // Do something...
        }
        task.resume()
    }
}
