//  DanceSession.swift
//  DanceHeatlhTracking WatchKit Extension

import WatchKit
import Foundation
import CoreMotion
import HealthKit
import CoreML

class DanceSession: WKInterfaceController, HKWorkoutSessionDelegate, HKLiveWorkoutBuilderDelegate {
    
// MARK: - Global variables & actions
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
    
    var avgHeartRate : Double = 0
    var minHeartRate : Double = 0
    var maxHeartRate : Double = 0
    var distanceTravelled : Double = 0
    var caloriesBurned : Double = 0
    
    final var SESSION_NOT_STARTED : Int = 0
    final var SESSION_STARTED : Int = 1
    final var DEVICE_UPDATE_INTERVAL  : Double = 0.3
    final var OPT_X_ROTATION : Double = 7.0
    final var OPT_Y_GRAVITY : Double = 0.2
    final var DEFAULT_VALUE : Int = -1
    
    @IBAction func startDancingButtonClick() {
        if(currentState == SESSION_NOT_STARTED){
            startSession()
            currentState = SESSION_STARTED
            startDancingButton.setTitle("Stop Measuring")
        }
        else{
            endSession()
            currentState = SESSION_NOT_STARTED
            startDancingButton.setTitle("Start Measuring")
        }
    }
    
    // MARK: - Interface Setup
    override func awake(withContext context: Any?) {
        super.awake(withContext: context)
        id = context as! String
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
        currentState = SESSION_STARTED
        startDancingButton.setTitle("Stop Dancing")
        motionData()
    }
        
    func endSession(){
        session.end()
        self.setDurationTimerDate(.ended)
        endCollectionOfWorkout()
        currentState = SESSION_NOT_STARTED
        startDancingButton.setTitle("Start Dancing")
        postData()
        motion.stopDeviceMotionUpdates()
        self.noOfSpinsCompleted = 0
    }
    
    //Used from SpeedySloth example code by Apple, with permission
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
            HKQuantityType.quantityType(forIdentifier: .heartRate)!,
            HKQuantityType.quantityType(forIdentifier: .distanceWalkingRunning)!,
            HKQuantityType.quantityType(forIdentifier: .activeEnergyBurned)!,
            HKQuantityType.quantityType(forIdentifier: .stepCount)!
        ]

        // Request authorization for those quantity types.
        healthStore.requestAuthorization(toShare: typesToShare, read: typesToRead) { (success, error) in
        }
    }
    
    func setUpEnv(){
        configuration = HKWorkoutConfiguration()
        configuration.activityType = .dance
        configuration.locationType = .indoor
    }
    
// MARK: - Workout Management
    func createHKWorkoutSession(){
        do {
            session = try HKWorkoutSession(healthStore: healthStore, configuration: configuration)
            builder = session.associatedWorkoutBuilder()
        } catch {
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
    //Sensors track the number of spins and update the interface
    func motionData(){
        motion.deviceMotionUpdateInterval = DEVICE_UPDATE_INTERVAL
        motion.startDeviceMotionUpdates(to: OperationQueue.current!) { (data, error) in
            if let deviceData = data {
                if(abs(deviceData.rotationRate.x) > self.OPT_X_ROTATION && abs(deviceData.gravity.y) < self.OPT_Y_GRAVITY){
                    self.noOfSpinsCompleted += 1
                }
                DispatchQueue.main.async {
                    self.spinsLabel.setText("Spins: \(self.noOfSpinsCompleted)")
                }
            }
        }
    }

    // MARK: - HKLiveWorkoutBuilderDelegate
    func workoutBuilder(_ workoutBuilder: HKLiveWorkoutBuilder, didCollectDataOf collectedTypes: Set<HKSampleType>) {
            
        //We are updating the interface with new data from the sensors
        DispatchQueue.main.async {
            if let heartRateType = HKQuantityType.quantityType(forIdentifier: .heartRate), let statistics = workoutBuilder.statistics(for: heartRateType){
                self.avgHeartRate = self.updateHeartRate(statistics: statistics)
                self.heartRateLabel.setText("\(self.avgHeartRate) BPM")
            }
        
            if let distanceType = HKQuantityType.quantityType(forIdentifier: .distanceWalkingRunning), let statistics = workoutBuilder.statistics(for: distanceType){
                self.distanceTravelled = self.updateDistance(statistics: statistics)
                self.distanceLabel.setText("\(self.distanceTravelled) m")
            }
            
            if let calorieType = HKQuantityType.quantityType(forIdentifier: .activeEnergyBurned), let statistics = workoutBuilder.statistics(for: calorieType){
                self.caloriesBurned = self.updateCalories(statistics: statistics)
                self.caloriesLabel.setText("\(self.caloriesBurned) cal")
            }
        }
    }
        
    func updateHeartRate(statistics : HKStatistics) -> Double{
        let heartRateUnit = HKUnit.count().unitDivided(by: HKUnit.minute())
        let minValue = statistics.minimumQuantity()?.doubleValue(for: heartRateUnit)
        self.minHeartRate = Double( round( 1 * minValue! ) / 1 )
        
        let maxValue = statistics.maximumQuantity()?.doubleValue(for: heartRateUnit)
        self.maxHeartRate = Double( round( 1 * maxValue! ) / 1 )
        
        let avgValue = statistics.mostRecentQuantity()?.doubleValue(for: heartRateUnit)
        return Double( round( 1 * avgValue! ) / 1 )
    }
    
    func updateDistance(statistics : HKStatistics) -> Double{
        let meterUnit = HKUnit.meter()
        let value = statistics.sumQuantity()?.doubleValue(for: meterUnit)
        return Double( round( 1 * value! ) / 1 )
    }
    
    func updateCalories(statistics : HKStatistics) -> Double{
        let energyUnit = HKUnit.kilocalorie()
        let value = statistics.sumQuantity()?.doubleValue(for: energyUnit)
        return Double( round( 1 * value! ) / 1 )
    }
        
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
    //Delegate methods
    func workoutSession(
        _ workoutSession: HKWorkoutSession,
        didChangeTo toState: HKWorkoutSessionState,
        from fromState: HKWorkoutSessionState,
        date: Date
    ) {
    }
    
    func workoutSession(_ workoutSession: HKWorkoutSession, didFailWithError error: Error) {
    }

// MARK: - URL Request
    func getDate() -> String{
        let today = Date()
        let formatter = DateFormatter()
        formatter.dateFormat = "dd-MM-yyyy"
        return formatter.string(from: today)
    }
    
    func createJSON() -> [String: Any]{
        return
            ["id": id,
            "duration": String(format: "%.01f", self.builder.elapsedTime),
            "minHeartRate": minHeartRate,
            "maxHeartRate": maxHeartRate,
            "averageHeartRate": avgHeartRate,
            "caloriesBurned": caloriesBurned,
            "distanceTravelled": distanceTravelled,
            "twists" : noOfSpinsCompleted,
            "timeStamp" : getDate(),
            "kinectDurationInSec" : DEFAULT_VALUE,
            "avgJointDistanceMoved" : DEFAULT_VALUE,
            "maxLeftHandHeight" : DEFAULT_VALUE,
            "maxRightHandHeight" : DEFAULT_VALUE,
            "maxLeftKneeHeight" : DEFAULT_VALUE,
            "maxRightKneeHeight": DEFAULT_VALUE
        ]
    }
    
    //Mongo DB Docs
    //https://docs.mongodb.com/stitch/services/http-actions/http.post/, Date Accessed: 27/01/2020
    func postData(){
        let jsonDanceObject: [String: Any] = createJSON()
        
        let jsonDanceObjectSerialized = try? JSONSerialization.data(withJSONObject: jsonDanceObject)
        
        //https://learnappmaking.com/urlsession-swift-networking-how-to/, Date Accessed: 02/02/2020
        let session = URLSession.shared
        let url = URL(string: "http://51.11.52.98:3300/dance")!
        var request = URLRequest(url: url)
        request.httpMethod = "POST"
        request.httpBody = jsonDanceObjectSerialized
        request.setValue("application/json", forHTTPHeaderField: "Content-Type")

        let task = session.uploadTask(with: request, from: jsonDanceObjectSerialized) { data, response, error in
        }
        task.resume()
    }
}
