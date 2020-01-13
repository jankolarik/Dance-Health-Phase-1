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


class InterfaceController: WKInterfaceController {
    
// MARK: - Declaration of variables + actions
    @IBOutlet weak var timer: WKInterfaceTimer!
    @IBOutlet weak var xLabel: WKInterfaceLabel!
    @IBOutlet weak var yLabel: WKInterfaceLabel!
    @IBOutlet weak var zLabel: WKInterfaceLabel!
    @IBOutlet weak var startDancingButton: WKInterfaceButton!
    
    let healthStore = HKHealthStore()
    let motion = CMMotionManager()
    var session: HKWorkoutSession!
    var builder: HKLiveWorkoutBuilder!
    var configuration: HKWorkoutConfiguration!
    
    var currentState : Int = 0
    var noOfTurnsCompleted : Int = 0
    var isAutorotating : Bool = true
    
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
        print("Start")
        createHKWorkoutSession()
        currentState = 1
        startDancingButton.setTitle("Stop Dancing")
        motionData()
        session.startActivity(with: Date())
        builder.beginCollection(withStart: Date()) { (success, error) in
            self.setDurationTimerDate(.running)
        }
    }
        
    func endSession(){
        session.end()
        self.setDurationTimerDate(.ended)
        builder.endCollection(withEnd: Date()) { (success, error) in
            self.builder.finishWorkout { (workout, error) in
                // Dispatch to main, because we are updating the interface.
                DispatchQueue.main.async() {
                    self.dismiss()
                }
            }
        }
        currentState = 0
        print("No of turns: = \(self.noOfTurnsCompleted)")
        print("Stop")
        motionData()
        startDancingButton.setTitle("Start Dancing")
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
            HKQuantityType.quantityType(forIdentifier: .heartRate)!,
            HKQuantityType.quantityType(forIdentifier: .activeEnergyBurned)!,
            HKQuantityType.quantityType(forIdentifier: .distanceWalkingRunning)!,
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
            dismiss()
            return
        }
        
        builder.dataSource = HKLiveWorkoutDataSource(healthStore: healthStore,
        workoutConfiguration: configuration)
    }
    
    
    // MARK: - Motion Manager

    //https://stackoverflow.com/questions/46756274/how-to-work-with-accelerometer-data-from-the-apple-watch
    
    func motionData(){
        if(self.currentState == 1){
            motion.deviceMotionUpdateInterval = 0.3

            motion.startDeviceMotionUpdates(to: OperationQueue.current!) { (data, error) in
                if let myData = data {
                    self.xLabel.setText(String(myData.userAcceleration.x))
                    self.yLabel.setText(String(myData.userAcceleration.y))
                    self.zLabel.setText(String(myData.userAcceleration.z))
                     //print("x: \(myData.acceleration.x) y: \(myData.acceleration.y) z: \(myData.acceleration.z)")
                    /*if abs(myData.userAcceleration.y) > 0.05 && abs(myData.userAcceleration.z) > 0.05 {
                        print("Turn")
                        self.noOfTurnsCompleted += 1
                    }*/
                    if(abs(myData.rotationRate.x) > 7){
                        self.noOfTurnsCompleted += 1
                        print("Turn \(self.noOfTurnsCompleted)")
                    }
                }
            }
        }
        else{
            motion.stopDeviceMotionUpdates()
            return
        }
    }
    
    // MARK: - Machine Learning Model
    //https://github.com/skafos/ActivityClassifier/blob/master/ActivityClassifier/ViewController.swift
    //https://developer.apple.com/documentation/coreml/integrating_a_core_ml_model_into_your_app
    //https://developer.apple.com/documentation/createml/mlactivityclassifier
    let model = DanceActivityTest_1()
    

    
    struct MLActivityClassifier{
        var accelerometerAccelerationX_G_ : Double
        var accelerometerAccelerationY_G_ : Double
        var accelerometerAccelerationZ_G_ : Double
        var gyroRotationX_rad_s_ : Double
        var gyroRotationY_rad_s_ : Double
        var gyroRotationZ_rad_s_ : Double
    }
    
    //let input = DanceActivityTest_1Input.init(accelerometerAccelerationX_G_: MLMultiArray, accelerometerAccelerationY_G_: MLMultiArray, accelerometerAccelerationZ_G_: MLMultiArray, gyroRotationX_rad_s_: MLMultiArray, gyroRotationY_rad_s_: <#T##MLMultiArray#>, gyroRotationZ_rad_s_: <#T##MLMultiArray#>)
    
    let output = DanceActivityTest_1Output.init(labelProbability: <#T##[String : Double]#>, label: <#T##String#>, stateOut: <#T##MLMultiArray#>)
    
    
    var lastHiddenOutput = try? MLMultiArray(shape: [200 as NSNumber], dataType: MLMultiArrayDataType.double)
    var lastHiddenCellOutput = try? MLMultiArray(shape: [200 as NSNumber], dataType: MLMultiArrayDataType.double)
    let predictionWindowDataArray = try? MLMultiArray(shape: [1, 200, 6] as [NSNumber], dataType: MLMultiArrayDataType.double)

    
    guard let dataArray = predictionWindowDataArray else { return "Error!" }
        
    let modelPrediction = try? model.prediction(features: dataArray, hiddenIn: lastHiddenOutput, cellIn: lastHiddenCellOutput)
    
    lastHiddenOutput = modelPrediction?.hiddenOut
    lastHiddenCellOutput = modelPrediction?.cellOut

    
    // MARK: - Delete (Not Used)

    // https://developer.apple.com/documentation/coremotion/getting_raw_accelerometer_events
        
    func startAccelerometers() {
       // Make sure the accelerometer hardware is available.
       if self.motion.isAccelerometerAvailable {
          self.motion.accelerometerUpdateInterval = 1.0 / 60.0  // 60 Hz
          self.motion.startAccelerometerUpdates()

          // Configure a timer to fetch the data.
          /*self.timer = Timer(fire: Date(), interval: (1.0/60.0),
                repeats: true, block: { (timer) in
             // Get the accelerometer data.
             if let data = self.motion.accelerometerData {
                let x = data.acceleration.x
                let y = data.acceleration.y
                let z = data.acceleration.z

                // Use the accelerometer data in your app.
             }
          })

          // Add the timer to the current run loop.
          RunLoop.current.add(self.timer!, forMode: .defaultRunLoopMode)*/
       }
    }
    
    //https://nshipster.com/cmdevicemotion/
    func measure(){
        sleep(500)
        guard motion.isAccelerometerAvailable else {
            return
        }
        motion.startAccelerometerUpdates()
        if(!motion.isAccelerometerActive){
            return
        }
        xLabel.setText(String(motion.accelerometerData!.acceleration.x))
        yLabel.setText(String(motion.accelerometerData!.acceleration.y))
        zLabel.setText(String(motion.accelerometerData!.acceleration.z))
    }
}
