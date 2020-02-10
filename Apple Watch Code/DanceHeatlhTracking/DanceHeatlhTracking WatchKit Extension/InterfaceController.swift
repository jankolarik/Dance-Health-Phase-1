//
//  InterfaceController.swift
//  DanceHeatlhTracking WatchKit Extension
//
//  Created by Honza on 06/01/2020.
//  Copyright © 2020 Team32. All rights reserved.
//

import WatchKit
import Foundation
import CoreMotion
import HealthKit
import CoreML


class InterfaceController: WKInterfaceController, HKWorkoutSessionDelegate, HKLiveWorkoutBuilderDelegate {
    
// MARK: - Declaration of variables + actions
    @IBOutlet weak var timer: WKInterfaceTimer!
    @IBOutlet weak var heartRateLabel: WKInterfaceLabel!
    @IBOutlet weak var caloriesLabel: WKInterfaceLabel!
    
    @IBOutlet weak var distanceLabel: WKInterfaceLabel!
    @IBOutlet weak var spinsLabel: WKInterfaceLabel!
    @IBOutlet weak var startDancingButton: WKInterfaceButton!
    
    let healthStore = HKHealthStore()
    let motion = CMMotionManager()
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
        }
        else{
            endSession()
        }
    }
    
    // MARK: - Default Interface Setup
    override func awake(withContext context: Any?) {
        super.awake(withContext: context)
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
        session.startActivity(with: Date())
        builder.beginCollection(withStart: Date()) { (success, error) in
            self.setDurationTimerDate(.running)
        }
        
        // Setup session and builder.
        session.delegate = self
        builder.delegate = self
    }
        
    func endSession(){
        session.end()
        self.setDurationTimerDate(.ended)
        endCollectionOfWorkout()
        currentState = 0
        motionData()
        startDancingButton.setTitle("Start Dancing")
        //CREATE JSON AND SEND IT
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
            HKQuantityType.workoutType()
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
            dismiss()
            return
        }
        
        builder.dataSource = HKLiveWorkoutDataSource(healthStore: healthStore,
        workoutConfiguration: configuration)
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
            self.noOfSpinsCompleted = 0
            motion.stopDeviceMotionUpdates()
            return
        }
    }
    

    // MARK: - HKLiveWorkoutBuilderDelegate
    
    func workoutBuilder(_ workoutBuilder: HKLiveWorkoutBuilder, didCollectDataOf collectedTypes: Set<HKSampleType>) {
            
        let heartRateType = HKQuantityType.quantityType(forIdentifier: .heartRate)!
        if let statistics = workoutBuilder.statistics(for: heartRateType){
            let heartRateUnit = HKUnit.count().unitDivided(by: HKUnit.minute())
            let value = statistics.mostRecentQuantity()?.doubleValue(for: heartRateUnit)
            let roundedValue = Double( round( 1 * value! ) / 1 )
            heartRateLabel.setText("\(roundedValue) BPM")
            avgHeartRate = roundedValue
            
            let minValue = statistics.minimumQuantity()?.doubleValue(for: heartRateUnit)
            let minRoundedValue = Double( round( 1 * minValue! ) / 1 )
            minHeartRate = minRoundedValue
            
            let maxValue = statistics.maximumQuantity()?.doubleValue(for: heartRateUnit)
            let maxRoundedValue = Double( round( 1 * maxValue! ) / 1 )
            maxHeartRate = maxRoundedValue
        }
        
        let distanceType = HKQuantityType.quantityType(forIdentifier: .distanceWalkingRunning)!
        if let statistics = workoutBuilder.statistics(for: distanceType){
            let meterUnit = HKUnit.meter()
            let value = statistics.sumQuantity()?.doubleValue(for: meterUnit)
            let roundedValue = Double( round( 1 * value! ) / 1 )
            distanceLabel.setText("\(roundedValue) m")
            distanceTravelled = roundedValue
        }
        
        let calorieType = HKQuantityType.quantityType(forIdentifier: .activeEnergyBurned)!
        if let statistics = workoutBuilder.statistics(for: calorieType){
            let energyUnit = HKUnit.kilocalorie()
            let value = statistics.sumQuantity()?.doubleValue(for: energyUnit)
            let roundedValue = Double( round( 1 * value! ) / 1 )
            caloriesLabel.setText("\(roundedValue) cal")
            caloriesBurned = roundedValue
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
        DispatchQueue.main.async {
            //self.setupMenuItemsForWorkoutSessionState(toState)
        }
    }
    
    func workoutSession(_ workoutSession: HKWorkoutSession, didFailWithError error: Error) {
        // No error handling in this sample project.
    }
    
    // MARK: - JSON Object
    func createJSON(){
        let jsonObject: [String: Any] = [
            "session_id": 0,
            "minHeartRate": minHeartRate,
            "maxHeartRate": maxHeartRate,
            "averageHeartRate": avgHeartRate,
            "caloriesBurned": caloriesBurned,
            "distanceTravelled": distanceTravelled
        ]

        if(JSONSerialization.isValidJSONObject(jsonObject)){
        }
        else{
            //Error Handling
        }
    }
    
    
}

// MARK: - Creating URL Requests
    //https://www.youtube.com/watch?v=BFaZaUTF6m4
    

/*func getURLRequests(){
    let session = URLSession(configuration: .ephemeral)
    
    var url = URL(string: "https://dancehealth.azurewebsites.net/users")
    var request = URLRequest(url : url!)

    request.cachePolicy = .reloadIgnoringLocalAndRemoteCacheData
    request.networkServiceType = .background
    
    //Part 3
    request.allowsCellularAccess = false
    
    //Part 4
    let taskWithRequest = session.dataTask(with: request)
    
    //Part 5
    taskWithRequest.currentRequest?.httpMethod
    
    //Part 6
    request.httpMethod = "POST"
    
    //Part 7
    request.addValue("application/json", forHTTPHeaderField: "content-type")
    
    //Part 8
    struct Post: Codable {
        //let id: Int
        let author: String
        let title: String
    }
    
    let encoder = JSONEncoder()
    let post = Post(author: "Example Author", title: "Example Title")
    do {
        let data = try encoder.encode(post)
        request.httpBody = data
    } catch let encoderError as NSError {
        //print("Encoder error: \(encodeError.localizedError.localizedDescription)")
    }
    
    //Part 9
    taskWithRequest.currentRequest?.httpMethod
    
    //Part 10
    let postTask = session.dataTask(with: request){ data, response, error in
        defer{}
        guard let data = data, let response = response as? HTTPURLResponse, response.statusCode == 201 else{
            print("No data or status code not created")
            return
        }
    
    
        let decoder = JSONDecoder()
        do {
            let post = try decoder.decode(Post.self, from: data)
            post
        } catch let decodeError as NSError {
            //Print
            return
        }
    }
    
    //Part 11
    postTask.currentRequest?.httpMethod
    postTask.currentRequest?.allHTTPHeaderFields
    postTask.currentRequest?.httpBody

    //Part 12
    postTask.resume()
    
    //Part 13
    postTask.currentRequest?.httpBody
}*/



